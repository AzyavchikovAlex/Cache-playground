#pragma once

#include <cassert>
#include <unordered_map>
#include <format>

#include "cache_interface.h"

template<typename K>
class LFUCache : public Cache<K> {
 private:

  struct CounterInfo {
    int64_t frequency{0};
    std::list<K> keys;
  };
  std::list<CounterInfo> counters_;
  struct KeyInfo {
    typename std::list<CounterInfo>::iterator counter;
    typename std::list<K>::iterator position;
  };
  std::unordered_map<K, KeyInfo> keys_info_;
  int64_t zero_counter_{0};
  size_t max_size_{0};
  size_t size_{0};

  void TouchInternal(auto info_it) {
    assert(info_it != keys_info_.end());
    auto& info = info_it->second;
    int64_t new_frequency = info.counter->frequency + 1;
    assert(new_frequency > zero_counter_);

    auto insert_pos = std::next(info.counter);
    if (insert_pos == counters_.end()
        || insert_pos->frequency > new_frequency) {
      insert_pos = counters_.insert(insert_pos, CounterInfo{
          .frequency = new_frequency,
          .keys = std::list<K>{},
      });
    }
    insert_pos->keys.splice(insert_pos->keys.begin(),
                            info.counter->keys,
                            info.position);
    if (info.counter->keys.empty()) {
      counters_.erase(info.counter);
    }
    info.counter = insert_pos;
  }

 public:
  explicit LFUCache(size_t size) : max_size_(size) {
    if (size < 1) {
      throw std::runtime_error{std::format("Too small size (= {})", size)};
    }
  }

  ~LFUCache() override = default;

  K EvictKey() {
    assert(!counters_.empty() && !counters_.front().keys.empty());

    K evicted_key = counters_.front().keys.back();
    zero_counter_ = std::max(zero_counter_, counters_.front().frequency);
    counters_.front().keys.pop_back();
    if (counters_.front().keys.empty()) {
      counters_.pop_front();
    }
    keys_info_.erase(evicted_key);
    --size_;
    return evicted_key;
  }

  [[nodiscard]] size_t Size() const {
    return size_;
  }

  void Resize(size_t new_size) {
    if (new_size == 0) {
      throw std::runtime_error{"Zero new_size for LFU cache is invalid"};
    }
    while (Size() > new_size) {
      EvictKey();
    }
    max_size_ = new_size;
  }

  void Insert(const K& key) override {
    if (auto it = keys_info_.find(key); it != keys_info_.end()) {
      TouchInternal(it);
      return;
    }

    auto counter_insert = [this, &key](auto insert_pos) {
      if (insert_pos == counters_.end() ||
          insert_pos->frequency > zero_counter_ + 1) {
        insert_pos = counters_.insert(insert_pos, CounterInfo{
            .frequency = zero_counter_ + 1,
            .keys = std::list<K>{},
        });
      }
      assert(insert_pos->frequency == zero_counter_ + 1);

      insert_pos->keys.push_front(key);
      keys_info_.insert({key, KeyInfo{
          .counter = insert_pos,
          .position = insert_pos->keys.begin(),
      }});
      ++size_;
    };

    if (Size() + 1 > max_size_) {
      EvictKey();
    }
    if (counters_.empty() ||
        counters_.front().frequency >= zero_counter_ + 1) {
      counter_insert(counters_.begin());
    } else {
      counter_insert(std::next(counters_.begin()));
    }
  }

  void Touch(const K& key) override {
    if (auto it = keys_info_.find(key); it != keys_info_.end()) {
      TouchInternal(it);
      return;
    }
    throw std::runtime_error{std::format("Key {} is not in cache", key)};
  }

  bool Contains(const K& key) const override {
    return keys_info_.contains(key);
  }
};
