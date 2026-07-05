#pragma once

#include <set>
#include <list>
#include <queue>
#include <format>
#include <exception>
#include <cassert>

#include "cache_interface.h"
#include "LRUCache.h"

template<typename K>
class LIRSCache : public Cache<K> {
 private:
  size_t max_cold_keys_count_{0};
  size_t max_hot_keys_count_{0};
  size_t hot_keys_count{0};
  enum class KeyState {
    LIR, // Low Inter-reference Recency
    HIR, // High Inter-reference Recency
    DELETED,
  };

  struct StackNode {
    K key;
    KeyState state; // for stack pruning
  };

  struct KeyInfo {
    KeyState state;
    std::optional<typename std::list<StackNode>::iterator> stack_it;
    std::optional<typename std::list<K>::iterator> queue_it;
  };

  std::unordered_map<K, KeyInfo> keys_info_;
  std::list<K> hirs_queue_;
  std::list<StackNode> lirs_stack_;

  void PruneStack() {
    while (!lirs_stack_.empty() && lirs_stack_.back().state != KeyState::LIR) {
      auto info_it = keys_info_.find(lirs_stack_.back().key);
      auto& info = info_it->second;
      info.stack_it = std::nullopt;
      assert((info.queue_it.has_value() && info.state == KeyState::HIR) ||
          (!info.queue_it.has_value() && info.state == KeyState::DELETED));
      if (!info.queue_it.has_value()) {
        keys_info_.erase(info_it);
      }
      lirs_stack_.pop_back();
    }
  }

  void DeleteHIRKey() {
    auto info_it = keys_info_.find(hirs_queue_.back());
    auto& info = info_it->second;
    info.queue_it = std::nullopt;
    hirs_queue_.pop_back();

    assert(info.state == KeyState::HIR);
    if (info.stack_it.has_value()) {
      auto& node = *(info.stack_it.value());
      node.state = KeyState::DELETED;
      info.state = KeyState::DELETED;
    } else {
      keys_info_.erase(info_it);
    }
  }

  void DeleteLIRKey() {
    auto info_it = keys_info_.find(lirs_stack_.back().key);
    auto& info = info_it->second;
    assert(lirs_stack_.back().state == KeyState::LIR
               && info.state == KeyState::LIR);

    info.stack_it = std::nullopt;
    lirs_stack_.pop_back();
    hirs_queue_.push_front(info_it->first);
    info.queue_it = hirs_queue_.begin();
    info.state = KeyState::HIR;
    --hot_keys_count;

    if (hirs_queue_.size() > max_cold_keys_count_) {
      assert(hirs_queue_.size() == max_cold_keys_count_ + 1);
      DeleteHIRKey();
    }
    PruneStack();
  }

  void TouchInternal(auto key_info_it) {
    assert(key_info_it != keys_info_.end());

    auto& info = key_info_it->second;
    if (!info.stack_it.has_value()) {
      assert(info.state == KeyState::HIR);
      assert(info.queue_it.has_value());
      hirs_queue_.splice(hirs_queue_.begin(),
                         hirs_queue_,
                         info.queue_it.value());

      lirs_stack_.push_front(StackNode{
        .key = key_info_it->first,
        .state = KeyState::HIR,
      });
      info.stack_it = lirs_stack_.begin();
      return;
    }

    lirs_stack_.splice(lirs_stack_.begin(), lirs_stack_, info.stack_it.value());
    if (info.state != KeyState::LIR) {
      // become LIR
      if (info.state == KeyState::HIR) {
        assert(info.queue_it.has_value());
        hirs_queue_.erase(info.queue_it.value());
        info.queue_it = std::nullopt;
      }

      assert(!info.queue_it.has_value());
      info.state = KeyState::LIR;
      StackNode& node = *info.stack_it.value();
      node.state = KeyState::LIR;
      ++hot_keys_count;
      if (hot_keys_count > max_hot_keys_count_) {
        assert(hot_keys_count == max_hot_keys_count_ + 1);
        DeleteLIRKey();
      }
    }
    PruneStack();
    assert(hot_keys_count <= max_hot_keys_count_
               && hirs_queue_.size() <= max_cold_keys_count_);
  }

 public:
  explicit LIRSCache(size_t size) :
      max_cold_keys_count_(std::max(size_t(1),
                                    size_t(size * 0.1))),
      max_hot_keys_count_(size - max_cold_keys_count_) {
    if (size < 2) {
      throw std::runtime_error{
          std::format("Too small (= {}) size for LIRS cache", size)};
    }
  }

  virtual ~LIRSCache() = default;

  void Insert(const K& key) override {
    if (auto info_it = keys_info_.find(key); info_it != keys_info_.end()) {
      TouchInternal(info_it);
      return;
    }

    if (hot_keys_count < max_hot_keys_count_) {
      lirs_stack_.push_front(StackNode{
          .key = key,
          .state = KeyState::LIR,
      });
      keys_info_.insert({key, KeyInfo{
          .state = KeyState::LIR,
          .stack_it = lirs_stack_.begin(),
          .queue_it = std::nullopt,
      }});
      ++hot_keys_count;
      return;
    }

    lirs_stack_.push_front(StackNode{
        .key = key,
        .state = KeyState::HIR,
    });
    hirs_queue_.push_front(key);
    keys_info_.insert({key, KeyInfo{
        .state = KeyState::HIR,
        .stack_it = lirs_stack_.begin(),
        .queue_it = hirs_queue_.begin(),
    }});
    if (hirs_queue_.size() > max_cold_keys_count_) {
      assert(hirs_queue_.size() == max_cold_keys_count_ + 1);
      DeleteHIRKey();
    }

    assert(hot_keys_count <= max_hot_keys_count_
               && hirs_queue_.size() <= max_cold_keys_count_);
  }

  void Touch(const K& key) override {
    auto info_it = keys_info_.find(key);

    if (info_it == keys_info_.end()) {
      throw std::runtime_error{std::format("Key {} is not in cache", key)};
    }
    TouchInternal(info_it);
  }

  bool Contains(const K& key) const override {
    auto info_it = keys_info_.find(key);
    return info_it != keys_info_.end()
        && info_it->second.state != KeyState::DELETED;
  }
};