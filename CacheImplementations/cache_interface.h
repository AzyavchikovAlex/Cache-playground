#pragma once

#include <memory>
#include <variant>

template<typename K>
class Cache {
 public:
  using KeyType = K;

  virtual ~Cache() = default;

  virtual void Insert(const K& key) = 0;
  virtual void Touch(const K& key) = 0;
  [[nodiscard]] virtual bool Contains(const K& key) const = 0;
};

using AnyCache = std::variant<
    std::shared_ptr<Cache<int64_t>>,
    std::shared_ptr<Cache<std::string>>
>;
