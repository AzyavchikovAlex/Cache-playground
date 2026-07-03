#pragma once

#include <vector>
#include <variant>
#include <string>

template <typename T>
struct Dataset {
  std::vector<T> keys;
};

using AnyDataset = std::variant<Dataset<int64_t>, Dataset<std::string>>;