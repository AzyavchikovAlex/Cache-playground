#pragma once

#include <cassert>
#include <unordered_map>
#include <format>

#include "cache_interface.h"

template<typename K>
class LFUCache : public Cache<K> {
 private:
  struct BucketInfo {
    int64_t frequency{0};
    std::list<K> keys;
  };
  struct KeyInfo {
    typename std::list<BucketInfo>::iterator bucket;
    typename std::list<K>::iterator position;
  };
  std::list<BucketInfo> buckets_;
  std::unordered_map<K, KeyInfo> keys_info_;
  int64_t zero_frequency_{0};
  size_t max_size_{0};
  size_t size_{0};

  void TouchInternal(auto info_it) {
    assert(info_it != keys_info_.end());
    auto& info = info_it->second;
    int64_t new_frequency = info.bucket->frequency + 1;
    assert(new_frequency > zero_frequency_);

    // create (if needed) next bucket and move to it
    auto destination_bucket = std::next(info.bucket);
    if (destination_bucket == buckets_.end() ||
        destination_bucket->frequency > new_frequency) {
      destination_bucket = buckets_.insert(destination_bucket, BucketInfo{
          .frequency = new_frequency,
          .keys = std::list<K>{},
      });
    }
    destination_bucket->keys.splice(destination_bucket->keys.begin(),
                                    info.bucket->keys,
                                    info.position);

    // clear prev bucket if empty
    if (info.bucket->keys.empty()) {
      buckets_.erase(info.bucket);
    }
    info.bucket = destination_bucket;
  }

 public:
  explicit LFUCache(size_t size) : max_size_(size) {
    if (size < 1) {
      throw std::runtime_error{
          std::format("Too small size (= {}) for LFU cache", size)};
    }
  }

  K EvictKey() {
    if (size_ == 0) {
      throw std::runtime_error{"Cannot evict key from empty cache"};
    }

    auto& target_bucket = buckets_.front();
    K evicted_key = target_bucket.keys.back();
    // when searching for the key to the eviction, all frequencies should be lowered
    zero_frequency_ = std::max(zero_frequency_, target_bucket.frequency);
    target_bucket.keys.pop_back();
    if (target_bucket.keys.empty()) {
      buckets_.pop_front();
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

    auto bucket_insert = [this, &key](auto insert_pos) {
      if (insert_pos == buckets_.end() ||
          insert_pos->frequency > zero_frequency_ + 1) {
        insert_pos = buckets_.insert(insert_pos, BucketInfo{
            .frequency = zero_frequency_ + 1,
            .keys = std::list<K>{},
        });
      }
      assert(insert_pos->frequency == zero_frequency_ + 1);

      insert_pos->keys.push_front(key);
      keys_info_.insert({key, KeyInfo{
          .bucket = insert_pos,
          .position = insert_pos->keys.begin(),
      }});
      ++size_;
    };

    if (Size() + 1 > max_size_) {
      EvictKey();
    }

    if (!buckets_.empty() && buckets_.front().frequency <= zero_frequency_) {
      assert(buckets_.front().frequency == zero_frequency_);
      bucket_insert(std::next(buckets_.begin()));
    } else {
      bucket_insert(buckets_.begin());
    }
  }

  void Touch(const K& key) override {
    if (auto it = keys_info_.find(key); it != keys_info_.end()) {
      TouchInternal(it);
    } else {
      throw std::runtime_error{std::format("Key {} is not present", key)};
    }
  }

  bool Contains(const K& key) const override {
    return keys_info_.contains(key);
  }
};
