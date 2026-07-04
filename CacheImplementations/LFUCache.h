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
    if (size == 0) {
      throw std::runtime_error{"Zero size for LFU cache is invalid"};
    }
  }

  ~LFUCache() override = default;

  K EvictKey() {
    auto [info, key_to_erase] = *queue_.begin();
    EraseInternal(key_to_erase);
    return key_to_erase;
  }

  [[nodiscard]] size_t Size() const {
    return queue_.size();
  }

  void Resize(size_t new_size) {
    if (new_size == 0) {
      throw std::runtime_error{"Zero new_size for LFU cache is invalid"};
    }
    while (queue_.size() > new_size) {
      EvictKey();
    }
    size_ = new_size;
  }

  void Insert(const K& key) override {
    if (Contains(key)) {
      Touch(key);
      return;
    }
    if (queue_.size() >= size_) {
      EvictKey();
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

  bool Contains(const K& key) const override {
    return keys_info.contains(key);
  }
};
