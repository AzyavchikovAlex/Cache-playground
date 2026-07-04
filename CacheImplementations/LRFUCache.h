#pragma once

#include <format>

#include "cache_interface.h"
#include "LRUCache.h"
#include "LFUCache.h"

template<typename K>
class LRFUCache : public Cache<K> {
  LRUCache<K> lru_cache_;
  LFUCache<K> lfu_cache_;
 public:

  explicit LRFUCache(size_t size) : lru_cache_(size / 2),
                                    lfu_cache_(size - (size / 2)) {
    if (size < 2) {
      throw std::runtime_error{
          std::format("Too small size ({}) for cache", size)};
    }
  }
  virtual ~LRFUCache() = default;

  void Insert(const K& key) override {
    if (lfu_cache_.Contains(key)) {
      lfu_cache_.Touch(key);
    } else if (lru_cache_.Contains(key)) {
      lru_cache_.Erase(key);
      auto erased_key = lfu_cache_.EraseLeastFrequentlyUsed();
      lfu_cache_.Insert(key);
      lru_cache_.Insert(erased_key);
    } else {
      lru_cache_.Insert(key);
    }
  }

  void Touch(const K& key) override {
    if (lru_cache_.Contains(key)) {
      lru_cache_.Erase(key);
      lfu_cache_.Insert(key);
    } else {
      assert(lfu_cache_.Contains(key));
      lfu_cache_.Touch(key);
    }
  }

  bool Contains(const K& key) const override {
    return lru_cache_.Contains(key) || lfu_cache_.Contains(key);
  }
};