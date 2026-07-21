#pragma once

#include <cassert>
#include <format>
#include <list>
#include <stdexcept>
#include <unordered_map>

#include "cache_interface.h"

template<typename K>
class LRUPolicy : public KeyReplacementPolicy<K> {
 private:
  std::unordered_map<K, typename std::list<K>::iterator> keys_positions_;
  std::list<K> sorted_keys_;
  size_t max_size_{0};

  void EraseInternal(auto it) {
    if (it == keys_positions_.end()) {
      throw std::runtime_error{"Invalid iterator in erase function"};
    }
    this->evict_callback_(it->first);
    sorted_keys_.erase(it->second);
    keys_positions_.erase(it);
  }

  void TouchKeyInternal(auto it) {
    auto list_it = it->second;
    sorted_keys_.splice(sorted_keys_.begin(), sorted_keys_, list_it); // move to front
  }

 public:
  explicit LRUPolicy(size_t size) : max_size_(size) {
    if (size == 0) {
      throw std::runtime_error{"Zero size for LRU cache is invalid"};
    }
  }

  K EvictKey() {
    if (sorted_keys_.empty()) {
      throw std::runtime_error{"Cannot evict key from empty cache"};
    }
    K key_to_erase = *sorted_keys_.rbegin();
    EraseInternal(keys_positions_.find(key_to_erase));
    return key_to_erase;
  }

  [[nodiscard]] size_t Size() const {
    return sorted_keys_.size();
  }

  bool Erase(const K& key) {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      EraseInternal(it);
      return true;
    } else {
      return false;
    }
  }

  void Touch(const K& key) override {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      TouchKeyInternal(it);
      return;
    }

    assert(sorted_keys_.size() <= max_size_);
    if (sorted_keys_.size() == max_size_) {
      EvictKey();
    }

    sorted_keys_.push_front(key);
    keys_positions_[key] = sorted_keys_.begin();
  }

  bool Contains(const K& key) const override {
    return keys_positions_.contains(key);
  }

  void Resize(size_t new_size) override {
    if (new_size == 0) {
      throw std::runtime_error{"Zero new_size for LRU cache is invalid"};
    }
    while (sorted_keys_.size() > new_size) {
      EvictKey();
    }
    max_size_ = new_size;
  }
};


template<typename K>
class LRUCache : public Cache<K> {
 private:
  std::unordered_map<K, typename std::list<K>::iterator> keys_positions_;
  std::list<K> sorted_keys_;
  size_t max_size_{0};

  void EraseInternal(auto it) {
    if (it == keys_positions_.end()) {
      throw std::runtime_error{"Invalid iterator in erase function"};
    }
    sorted_keys_.erase(it->second);
    keys_positions_.erase(it);
  }

  void TouchKeyInternal(auto it) {
    auto list_it = it->second;
    sorted_keys_.splice(sorted_keys_.begin(), sorted_keys_, list_it); // move to front
  }

 public:
  explicit LRUCache(size_t size) : max_size_(size) {
    if (size == 0) {
      throw std::runtime_error{"Zero size for LRU cache is invalid"};
    }
  }

  K EvictKey() {
    if (sorted_keys_.empty()) {
      throw std::runtime_error{"Cannot evict key from empty cache"};
    }
    K key_to_erase = *sorted_keys_.rbegin();
    EraseInternal(keys_positions_.find(key_to_erase));
    return key_to_erase;
  }

  [[nodiscard]] size_t Size() const {
    return sorted_keys_.size();
  }

  void Resize(size_t new_size) {
    if (new_size == 0) {
      throw std::runtime_error{"Zero new_size for LRU cache is invalid"};
    }
    while (sorted_keys_.size() > new_size) {
      EvictKey();
    }
    max_size_ = new_size;
  }

  void Insert(const K& key) override {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      TouchKeyInternal(it);
      return;
    }
    
    assert(sorted_keys_.size() <= max_size_);
    if (sorted_keys_.size() == max_size_) {
      EvictKey();
    }

    sorted_keys_.push_front(key);
    keys_positions_[key] = sorted_keys_.begin();
  }

  bool Erase(const K& key) {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      EraseInternal(it);
      return true;
    } else {
      return false;
    }
  }

  void Touch(const K& key) override {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      TouchKeyInternal(it);
    } else {
      throw std::runtime_error{std::format("Key {} is not present", key)};
    }
  }

  bool Contains(const K& key) const override {
    return keys_positions_.contains(key);
  }
};
