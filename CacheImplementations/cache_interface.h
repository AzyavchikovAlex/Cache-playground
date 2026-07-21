#pragma once

#include <memory>
#include <variant>
#include <functional>

template<typename K>
class Cache {
 public:
  using KeyType = K;

  virtual ~Cache() = default;

  virtual void Insert(const K& key) = 0;
  virtual void Touch(const K& key) = 0;
  [[nodiscard]] virtual bool Contains(const K& key) const = 0;
};

template<typename K>
class NewCache {
 public:
  using KeyType = K;

  virtual ~NewCache() = default;
  virtual bool Get(const K& key) = 0;
  virtual void Set(const K& key) = 0;
};

template<typename K>
class KeyReplacementPolicy {
 protected:
  std::function<void(const K&)> evict_callback_{[](const K&) -> void {
    return;
  }};

 public:
  virtual ~KeyReplacementPolicy() = default;

  void SetEvictCallback(std::function<void(const K&)> callback) {
    evict_callback_ = std::move(callback);
  }

  virtual void Touch(const K& key) = 0;
  virtual bool Contains(const K& key) const = 0;

  virtual void Resize(size_t new_size) = 0;
};


using AnyCache = std::variant<
    std::shared_ptr<Cache<int64_t>>,
    std::shared_ptr<Cache<std::string>>,
    std::shared_ptr<NewCache<int64_t>>
>;
