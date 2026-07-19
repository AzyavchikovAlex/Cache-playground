#pragma once

#include <list>
#include <queue>
#include <format>
#include <exception>
#include <cassert>
#include <unordered_map>

#include "cache_interface.h"

template<typename K>
class LIRS2Cache : public Cache<K> {
 private:
  size_t max_hir_keys_count_{0};
  size_t max_lir_keys_count_{0};
  size_t lir_keys_count_{0};

  enum class KeyState {
    LIR,
    HIR,
    DELETED,
  };

  struct KeyInfo {
    KeyState state;

    std::optional<typename std::list<K>::iterator> stack_it;
    std::optional<typename std::list<K>::iterator> stack_second_it;
    std::optional<typename std::list<K>::iterator> queue_it;
  };

  std::unordered_map<K, KeyInfo> keys_info_;
  std::list<K> lir_stack_;
  std::list<K> hir_queue_;

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

  void PruneStack() {
    // cut tail
    while (!lir_stack_.empty()) {
      auto info_it = keys_info_.find(lir_stack_.back());
      auto& [key, info] = *info_it;

      if (info.state == KeyState::LIR &&
          lir_keys_count_ <= max_lir_keys_count_) {
        break;
      }

      // pop back
      if (info.stack_second_it.has_value()) {
        assert(*info.stack_second_it.value() == key);
        assert(info.stack_second_it.value() == std::prev(lir_stack_.end()));
        info.stack_second_it = std::nullopt;
      } else {
        assert(info.stack_it.value() == std::prev(lir_stack_.end()));
        info.stack_it = std::nullopt;
      }
      lir_stack_.pop_back();

      // update statuses
      if (info.state == KeyState::LIR) {
        assert(!info.queue_it.has_value());
        hir_queue_.push_front(key);
        info.queue_it = hir_queue_.begin();
        info.state = KeyState::HIR;
        --lir_keys_count_;
      } else if (info.state == KeyState::HIR) {
        assert(info.queue_it.has_value());
      } else if (info.state == KeyState::DELETED) {
        assert(info.queue_it == std::nullopt);
        if (!info.stack_it.has_value()) {
          // permanently deleted key
          keys_info_.erase(info_it);
        }
      }
    }
  }

  void TouchInternal(const K& key, KeyInfo& info) {
    lir_stack_.push_front(key);

    if (info.stack_second_it.has_value()) {
      if (info.state != KeyState::LIR) {
        // promote to LIR
        info.state = KeyState::LIR;
        ++lir_keys_count_;

        if (info.queue_it.has_value()) {
          hir_queue_.erase(info.queue_it.value());
          info.queue_it = std::nullopt;
        }
      }

      lir_stack_.erase(info.stack_second_it.value());
      info.stack_second_it = info.stack_it;
      info.stack_it = lir_stack_.begin();
    } else {
      info.state = KeyState::HIR;
      if (info.queue_it.has_value()) {
        hir_queue_.splice(hir_queue_.begin(),
                          hir_queue_,
                          info.queue_it.value());
      } else {
        hir_queue_.push_front(key);
        info.queue_it = hir_queue_.begin();
      }

      if (info.stack_it.has_value()) {
        info.stack_second_it = info.stack_it;
        info.stack_it = lir_stack_.begin();
      } else {
        info.stack_it = lir_stack_.begin();
      }
    }

    PruneQueue();
    PruneStack();
  }

 public:
  explicit LIRS2Cache(size_t size) :
      max_hir_keys_count_(std::max(size_t(1), size_t(size * 0.1))),
      max_lir_keys_count_(size - max_hir_keys_count_) {
    if (size < 2) {
      throw std::runtime_error{
          std::format("Too small size (= {}) for LIRS2Cache cache", size)};
    }
  }

  void Insert(const K& key) override {
    auto [it, is_inserted] = keys_info_.insert({key, KeyInfo{}});
    auto& info = it->second;
    if (!is_inserted) {
      TouchInternal(key, info);
      return;
    }

    lir_stack_.push_front(key);
    info.stack_it = lir_stack_.begin();

    if (lir_keys_count_ < max_lir_keys_count_) {
      info.state = KeyState::LIR;
      // invariant: "LIR key always has 2 stack positions"
      info.stack_second_it = info.stack_it;
      lir_stack_.push_front(key);
      info.stack_it = lir_stack_.begin();
      ++lir_keys_count_;
      return;
    }

    info.state = KeyState::HIR;
    hir_queue_.push_front(key);
    info.queue_it = hir_queue_.begin();
    PruneStack();
    PruneQueue();
  }

  // TODO: Touch function repeats in many implementations,
  //  think about writing keys meta-info storage
  void Touch(const K& key) override {
    auto info_it = keys_info_.find(key);

    if (info_it == keys_info_.end() ||
        info_it->second.state == KeyState::DELETED) {
      throw std::runtime_error{std::format("Key {} is not present", key)};
    }
    TouchInternal(info_it->first, info_it->second);
  }

  bool Contains(const K& key) const override {
    auto info_it = keys_info_.find(key);
    return info_it != keys_info_.end()
        && info_it->second.state != KeyState::DELETED;
  }
};
