#pragma once

#include <cassert>
#include <set>
#include <unordered_map>

#include "cache_interface.h"


template<typename K>
class LRUCache : public Cache<K> {
 private:
  size_t size_;
  int64_t timestamp_{0};
  std::unordered_map<K, int64_t> keys_timestamps; // key -> time
  std::set<std::pair<int64_t, K>> queue_; // timestamp, key

  void EraseInternal(const K& key) {
    auto time_it = keys_timestamps.find(key);
    assert(time_it != keys_timestamps.end());
    queue_.erase({time_it->second, key});
    keys_timestamps.erase(time_it);
  }

 public:
  explicit LRUCache(size_t size) : size_(size) {
  }

  ~LRUCache() override = default;

  void Insert(const K& key) override {
    if (IsPresent(key)) {
      return;
    }
    if (queue_.size() >= size_) {
      auto [timestamp, key_to_erase] = *queue_.begin();
      EraseInternal(key_to_erase);
    }

    int64_t key_timestamp = timestamp_++;
    keys_timestamps[key] = key_timestamp;
    queue_.insert({key_timestamp, key});
  }

  void Touch(const K& key) override {
    auto time_it = keys_timestamps.find(key);
    assert(time_it != keys_timestamps.end());
    queue_.erase({time_it->second, key});
    time_it->second = timestamp_++;
    queue_.insert({time_it->second, key});
  }

  bool IsPresent(const K& key) const override {
    return keys_timestamps.contains(key);
  }
};
