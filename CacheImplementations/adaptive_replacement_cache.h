#pragma once

#include <exception>
#include <format>

#include "cache_interface.h"
#include "LRUCache.h"

template<typename K>
class ARCache : public Cache<K> {
  size_t lru_max_size_;
  size_t second_chance_max_size_;
  LRUCache<K> lru_cache_;
  LRUCache<K> lru_cache_history_;
  LRUCache<K> second_chance_cache_;
  LRUCache<K> second_chance_cache_history_;

  void SetProperCacheSizes() {
    while (lru_cache_.Size() > lru_max_size_) {
      lru_cache_history_.Insert(lru_cache_.EvictKey());
    }
    lru_cache_.Resize(lru_max_size_);

    while (second_chance_cache_.Size() > second_chance_max_size_) {
      second_chance_cache_history_.Insert(second_chance_cache_.EvictKey());
    }
    second_chance_cache_.Resize(second_chance_max_size_);
  }

  void AdaptSizes(bool hit_in_lru) {
    if (hit_in_lru) {
      int delta = 1;
      if (lru_cache_history_.Size() > 0 && second_chance_cache_history_.Size() > lru_cache_history_.Size()) {
        delta = second_chance_cache_history_.Size() / lru_cache_history_.Size();
      }
      delta = std::min(delta, (int) second_chance_max_size_ - 1);
      lru_max_size_ += delta;
      second_chance_max_size_ -= delta;
    } else {
      int delta = 1;
      if (second_chance_cache_history_.Size() > 0 && lru_cache_history_.Size() > second_chance_cache_history_.Size()) {
        delta = lru_cache_history_.Size() / second_chance_cache_history_.Size();
      }
      delta = std::min(delta, (int) lru_max_size_ - 1);
      lru_max_size_ -= delta;
      second_chance_max_size_ += delta;
    }
    SetProperCacheSizes();
  }

  void LRUInsert(const K& key) {
    if (lru_cache_.Size() >= lru_max_size_) {
      assert(lru_cache_.Size() == lru_max_size_);
      lru_cache_history_.Insert(lru_cache_.EvictKey());
    }
    lru_cache_.Insert(key);
  }

  void SecondChanceInsert(const K& key) {
    if (second_chance_cache_.Size() >= second_chance_max_size_) {
      assert(second_chance_cache_.Size() == second_chance_max_size_);
      second_chance_cache_history_.Insert(second_chance_cache_.EvictKey());
    }
    second_chance_cache_.Insert(key);
  }

 public:
  explicit ARCache(size_t size) : lru_max_size_(size / 2),
                                  second_chance_max_size_(size - lru_max_size_),
                                  lru_cache_(lru_max_size_),
                                  lru_cache_history_(size),
                                  second_chance_cache_(second_chance_max_size_),
                                  second_chance_cache_history_(size) {
    if (size < 2) {
      throw std::runtime_error{
          std::format("Too small size (= {}) for ARCAche", size)};
    }
  }
  virtual ~ARCache() = default;

  void Insert(const K& key) override {
    if (lru_cache_.Contains(key)) {
      lru_cache_.Erase(key);
      SecondChanceInsert(key);
      return;
    }

    if (second_chance_cache_.Contains(key)) {
      second_chance_cache_.Touch(key);
      return;
    }

    bool seen_in_lru = lru_cache_history_.Contains(key);
    bool seen_in_second_chance = second_chance_cache_history_.Contains(key);
    if (seen_in_lru || seen_in_second_chance) {
      if (seen_in_lru && seen_in_second_chance) {
        throw std::runtime_error{"Unexpected branch"};
      }
      AdaptSizes(seen_in_lru);

      if (seen_in_lru) {
        lru_cache_history_.Erase(key);
      } else {
        second_chance_cache_history_.Erase(key);
      }
      SecondChanceInsert(key);
      return;
    }

    LRUInsert(key);
  }

  void Touch(const K& key) override {
    if (lru_cache_.Contains(key)) {
      lru_cache_.Erase(key);
      SecondChanceInsert(key);
    } else {
      assert(second_chance_cache_.Contains(key));
      second_chance_cache_.Touch(key);
    }
  }

  bool Contains(const K& key) const override {
    return lru_cache_.Contains(key) || second_chance_cache_.Contains(key);
  }
};