#pragma once

#include <algorithm>
#include <cassert>
#include <format>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <queue>

#include "cache_interface.h"

// offline algo, that requires future knowledge in constructor
template<typename K>
class OPTCache : public Cache<K> {
 private:
  std::unordered_map<K, std::vector<int64_t>> future_accesses_;
  struct KeyInfo {
    int64_t next_access;
    K key;

    auto operator<=>(const KeyInfo&) const = default;
  };
  std::unordered_map<K, typename std::multiset<KeyInfo>::iterator> keys_info_;
  std::multiset<KeyInfo, std::greater<KeyInfo>> cache_;
  size_t max_size{0};

 private:

  void TouchInternal(const K& key) {
    auto times_it = future_accesses_.find(key);
    if (times_it == future_accesses_.end() || times_it->second.size() < 2) {
      throw std::runtime_error{"Invalid future provided in constructor"};
    }
    auto& times = times_it->second;
    times.pop_back();
    int64_t next_access = times.back();

    // erase from cache if present
    if (auto it = keys_info_.find(key); it != keys_info_.end()) {
      assert(it->second != cache_.end());
      cache_.erase(it->second);
    }

    // do nothing if the object has not been accessed for a long time
    if (cache_.size() == max_size
        && next_access >= cache_.begin()->next_access) {
      keys_info_.erase(key);
      return;
    }

    // prune cache
    if (cache_.size() >= max_size) {
      keys_info_.erase(cache_.begin()->key);
      cache_.erase(cache_.begin());
    }

    // insert
    auto cache_it = cache_.insert(KeyInfo{
        .next_access = next_access,
        .key = key,
    });
    keys_info_[key] = cache_it;
  }

 public:
  explicit OPTCache(size_t size,
                    const auto key_sequence_begin,
                    const auto key_sequence_end)
      : max_size(size) {
    if (size == 0) {
      throw std::runtime_error{"Zero size for OPT cache is invalid"};
    }
    int64_t time = 0;
    for (auto it = key_sequence_begin; it != key_sequence_end; ++it) {
      static_assert(std::is_same_v<K, std::remove_cvref_t<decltype(*it)>>,
                    "Iterator must point to the same type as T");

      if (auto
            [sequence_it, ok] = future_accesses_.insert({*it, {time}}); !ok) {
        sequence_it->second.push_back(time);
      }
      ++time;
    }

    for (auto& [key, times] : future_accesses_) {
      times.push_back(std::numeric_limits<int64_t>::max());
      std::ranges::reverse(times);
    }
  }

  // not always inserts into cache! (e.g., if the key is never touched in the future)
  void Insert(const K& key) override {
    TouchInternal(key);
  }

  void Touch(const K& key) override {
    TouchInternal(key);
  }

  bool Contains(const K& key) const override {
    return keys_info_.contains(key);
  }
};
