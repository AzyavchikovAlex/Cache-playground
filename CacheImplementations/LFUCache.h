#pragma once

#include <cassert>
#include <set>
#include <unordered_map>

#include "cache_interface.h"

template<typename K>
class LFUCache : public Cache<K> {
 private:
  struct KeyInfo {
    int64_t counter{0};
    int64_t timestamp{0};

    auto operator<=>(const KeyInfo& info) const = default;
  };

  size_t size_;
  int64_t timestamp_{0};
  std::unordered_map<K, KeyInfo> keys_info; // key -> info
  std::set<std::pair<KeyInfo, K>> queue_; // info, key

  void EraseInternal(const K& key) {
    auto info_it = keys_info.find(key);
    assert(info_it != keys_info.end());
    queue_.erase({info_it->second, key});
    keys_info.erase(info_it);
  }

 public:
  explicit LFUCache(size_t size) : size_(size) {
  }

  ~LFUCache() override = default;

  void Insert(const K& key) override {
    if (IsPresent(key)) {
      return;
    }
    if (queue_.size() >= size_) {
      auto [info, key_to_erase] = *queue_.begin();
      EraseInternal(key_to_erase);
    }

    KeyInfo info{
        .counter = 1,
        .timestamp = timestamp_++,
    };
    keys_info[key] = info;
    queue_.insert({info, key});
  }

  void Touch(const K& key) override {
    auto info_it = keys_info.find(key);
    assert(info_it != keys_info.end());
    queue_.erase({info_it->second, key});
    info_it->second.counter += 1;
    info_it->second.timestamp = timestamp_++;
    queue_.insert({info_it->second, key});
  }

  bool IsPresent(const K& key) const override {
    return keys_info.contains(key);
  }
};
