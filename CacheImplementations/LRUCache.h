#pragma once

#include <cassert>
#include <format>
#include <list>
#include <stdexcept>
#include <unordered_map>

#include "cache_interface.h"


template<typename K>
class LRUCache : public Cache<K> {
 private:
  std::unordered_map<K, typename std::list<K>::iterator> keys_positions_;
  std::list<K> sorted_keys_;
  size_t size_;

  void EraseInternal(typename std::unordered_map<K, typename std::list<K>::iterator>::iterator it) {
    if (it == keys_positions_.end()) {
      throw std::runtime_error{"Invalid iterator in erase function"};
    }
    sorted_keys_.erase(it->second);
    keys_positions_.erase(it);
  }

  void TouchKeyInternal(typename std::unordered_map<K, typename std::list<K>::iterator>::iterator it) {
    auto list_it = it->second;
    sorted_keys_.splice(sorted_keys_.begin(), sorted_keys_, list_it); // move to front
  }

 public:
  explicit LRUCache(size_t size) : size_(size) {
    if (size == 0) {
      throw std::runtime_error{"Zero size for LRU cache is invalid"};
    }
  }

  ~LRUCache() override = default;

  K EvictKey() {
    auto key_to_erase = *sorted_keys_.rbegin();
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
    size_ = new_size;
  }

  void Insert(const K& key) override {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      TouchKeyInternal(it);
      return;
    }

    if (sorted_keys_.size() >= size_) {
      EvictKey();
    }

    sorted_keys_.push_front(key);
    keys_positions_[key] = sorted_keys_.begin();
  }

  bool Erase(const K& key) {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      EraseInternal(it);
      return true;
    }
    return false;
  }

  void Touch(const K& key) override {
    if (auto it = keys_positions_.find(key); it != keys_positions_.end()) {
      TouchKeyInternal(it);
    } else {
      throw std::runtime_error{std::format("Key ({}) is not present", key)};
    }
  }

  bool Contains(const K& key) const override {
    return keys_positions_.contains(key);
  }
};
