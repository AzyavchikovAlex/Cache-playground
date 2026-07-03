#include "dataset_parser.h"

#include <fstream>
#include <filesystem>
#include <format>

namespace {
template <typename T>
Dataset<T> ReadDataset(std::ifstream& in, size_t size) {
  if (!in.is_open()) {
    throw std::runtime_error{"Input stream is not opened"};
  }
  Dataset<T> dataset;
  dataset.keys.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    T buffer;
    if (!(in >> buffer)) {
      throw std::runtime_error("Unexpected end of file while reading dataset elements");
    }
    dataset.keys.push_back(std::move(buffer));
  }
  return dataset;
}

}

AnyDataset ParseDataset(const std::string& path_str) {
  std::filesystem::path path(path_str);
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error{std::format("Invalid path: {}", path_str)};
  }

  std::ifstream in(path);
  std::string type;
  size_t size;
  if (!(in >> type >> size)) {
    throw std::runtime_error(std::format("Header parsing failed for file: {}", path_str));
  }

  if (type == "int") {
    return ReadDataset<int64_t>(in, size);
  } else if (type == "string") {
    return ReadDataset<std::string>(in, size);
  } else {
      throw std::runtime_error{std::format("Unknown type: {}", type)};
  }
}