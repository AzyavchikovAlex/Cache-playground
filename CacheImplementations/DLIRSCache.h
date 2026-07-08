#pragma once

#include <list>
#include <queue>
#include <format>
#include <exception>
#include <cassert>
#include <unordered_map>

#include "cache_interface.h"

template<typename K>
class DLIRSCache : public Cache<K> {
 private:
  size_t max_hir_keys_count_{0};
  size_t max_lir_keys_count_{0};
  size_t lir_keys_count_{0};
  enum class KeyState {
    LIR, // Low Inter-reference Recency
    HIR, // High Inter-reference Recency
    HIR_FROM_LIR,
    DELETED,
  };

  struct KeyInfo {
    KeyState state;
    std::optional<typename std::list<K>::iterator> stack_it;
    std::optional<typename std::list<K>::iterator> queue_it;
  };

  std::unordered_map<K, KeyInfo> keys_info_;
  std::list<K> hir_queue_;
  std::list<K> lir_stack_;

 private:
  void PruneQueue() {
    while (hir_queue_.size() > max_hir_keys_count_) {
      auto info_it = keys_info_.find(hir_queue_.back());
      auto& [key, info] = *info_it;

      info.queue_it = std::nullopt;
      info.state = KeyState::DELETED;
      hir_queue_.pop_back();

      if (!info.stack_it.has_value()) {
        keys_info_.erase(info_it);
      }
    }
  }

  // may insert too much values to queue
  void PruneStack() {
    while (!lir_stack_.empty()) {
      auto info_it = keys_info_.find(lir_stack_.back());
      auto& [key, info] = *info_it;

      if (info.state == KeyState::LIR &&
          lir_keys_count_ <= max_lir_keys_count_) {
        break;
      }

      info.stack_it = std::nullopt;
      lir_stack_.pop_back();

      if (info.state == KeyState::LIR) {
        hir_queue_.push_front(key);
        info.queue_it = hir_queue_.begin();
        info.state = KeyState::HIR_FROM_LIR;
        --lir_keys_count_;
      } else if (info.state == KeyState::HIR ||
          info.state == KeyState::HIR_FROM_LIR) {
        assert(info.queue_it.has_value());
      } else if (info.state == KeyState::DELETED) {
        assert(info.queue_it == std::nullopt);
        keys_info_.erase(info_it);
      }
    }
  }

  void RebalanceSizes(KeyState hit_state) {
    if (hit_state == KeyState::HIR_FROM_LIR) {
      // inc lir stack size for more hits
      if (max_hir_keys_count_ > 1) {
        --max_hir_keys_count_;
        ++max_lir_keys_count_;
      }
    } else if (hit_state == KeyState::DELETED) {
      // inc hir queue for more hits (need more time for deletion)
      if (max_lir_keys_count_ > 1) {
        --max_lir_keys_count_;
        ++max_hir_keys_count_;
      }
    }
  }

  void TouchInternal(const K& key, KeyInfo& info) {
    // put to front of stack
    bool was_in_stack = info.stack_it.has_value();
    if (was_in_stack) {
      lir_stack_.splice(lir_stack_.begin(), lir_stack_, info.stack_it.value());
    } else {
      lir_stack_.push_front(key);
      info.stack_it = lir_stack_.begin();
    }

    RebalanceSizes(info.state);

    if (was_in_stack && info.state != KeyState::LIR) {
      info.state = KeyState::LIR;
      ++lir_keys_count_;
    }

    if (info.queue_it.has_value()) {
      if (info.state == KeyState::LIR) {
        hir_queue_.erase(info.queue_it.value());
        info.queue_it = std::nullopt;
      } else {
        hir_queue_.splice(hir_queue_.begin(), hir_queue_, info.queue_it.value());
      }
    }

    PruneStack();
    PruneQueue();

    assert(lir_keys_count_ <= max_lir_keys_count_
               && hir_queue_.size() <= max_hir_keys_count_);
  }

 public:
  explicit DLIRSCache(size_t size) :
      max_hir_keys_count_(std::max(size_t(1), size_t(size * 0.1))),
      max_lir_keys_count_(size - max_hir_keys_count_) {
    if (size < 2) {
      throw std::runtime_error{
          std::format("Too small (= {}) size for DLIRSCache cache", size)};
    }
  }

  void Insert(const K& key) override {
    auto [it, ok] = keys_info_.insert({key, KeyInfo{}});
    auto& [_, info] = *it;
    if (!ok) {
      TouchInternal(key, info);
      return;
    }

    lir_stack_.push_front(key);
    info.stack_it = lir_stack_.begin();

    if (lir_keys_count_ < max_lir_keys_count_) {
      info.state = KeyState::LIR;
      ++lir_keys_count_;
      return;
    }

    info.state = KeyState::HIR;
    hir_queue_.push_front(key);
    info.queue_it = hir_queue_.begin();
    PruneQueue();
  }

  void Touch(const K& key) override {
    auto info_it = keys_info_.find(key);

    if (info_it == keys_info_.end() ||
        info_it->second.state == KeyState::DELETED) {
      throw std::runtime_error{std::format("Key {} is not in cache", key)};
    }
    TouchInternal(info_it->first, info_it->second);
  }

  bool Contains(const K& key) const override {
    auto info_it = keys_info_.find(key);
    return info_it != keys_info_.end()
        && info_it->second.state != KeyState::DELETED;
  }
};