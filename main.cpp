#include <iostream>
#include <format>

#include "CacheImplementations/LRUCache.h"
#include "CacheImplementations/LFUCache.h"
#include "Datasets/dataset_parser.h"
#include "metrics.h"

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    throw std::runtime_error{"Too few arguments"};
  }

  std::string cache_name = argv[1];
  std::string dataset_path = argv[2];

  if (cache_name != "lru" && cache_name != "lfu") {
    throw std::runtime_error{std::format("Unknown cache name: {}", cache_name)};
  }

  auto dataset = ParseDataset(dataset_path);

  for (size_t size = 1; size < 100; ++size) {

    auto cache_factory = [size, &cache_name]<typename T>() -> AnyCache {
      if (cache_name == "lru") {
        return std::shared_ptr<LRUCache<T>>(new LRUCache<T>(size));
      } else if (cache_name == "lfu") {
        return std::shared_ptr<LFUCache<T>>(new LFUCache<T>(size));
      }
      throw std::runtime_error{
          std::format("Unexpected cache name {}", cache_name)};
    };

    auto m = MeasureCache(dataset, cache_factory);
    auto accuracy = m.GetAccuracy();
    std::cout << size << "\t" << accuracy << "\n";

    if (std::abs(accuracy - 1) < 0.000001) {
      break;
    }
  }
  return 0;
}
