#include <iostream>
#include <format>

#include "CacheImplementations/LRUCache.h"
#include "CacheImplementations/LFUCache.h"
#include "CacheImplementations/LRFUCache.h"
#include "Datasets/dataset_parser.h"
#include "metrics.h"

const double kMaxCacheFraction = 0.05;
const size_t kMaxCacheSize = 5'000;
const double kSizeMultiplier = 1.5;

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    throw std::runtime_error{"Too few arguments"};
  }

  std::string cache_name = argv[1];
  std::string dataset_path = argv[2];

  if (cache_name != "lru" && cache_name != "lfu" && cache_name != "lrfu") {
    throw std::runtime_error{std::format("Unknown cache name: {}", cache_name)};
  }

  auto dataset = ParseDataset(dataset_path);
  size_t dataset_size = GetDatasetSize(dataset);

  for (size_t size = 4;
       size < kMaxCacheSize;
       size = std::max(size + 1, size_t(double(size) * kSizeMultiplier))) {

    auto cache_factory = [size, &cache_name]<typename T>() -> AnyCache {
      if (cache_name == "lru") {
        return std::shared_ptr<LRUCache<T>>(new LRUCache<T>(size));
      } else if (cache_name == "lfu") {
        return std::shared_ptr<LFUCache<T>>(new LFUCache<T>(size));
      } else if (cache_name == "lrfu") {
        return std::shared_ptr<LRFUCache<T>>(new LRFUCache<T>(size));
      }
      throw std::runtime_error{
          std::format("Unexpected cache name {}", cache_name)};
    };

    auto m = MeasureCache(dataset, cache_factory);
    double cache_fraction = double(size) / double(dataset_size);
    auto accuracy = m.GetAccuracy();
    std::cout << size << "\t" << cache_fraction << "\t" << accuracy << "\n";

    if (std::abs(accuracy - 1) < 0.000001 ||
        cache_fraction >= kMaxCacheFraction) {
      break;
    }
  }
  return 0;
}
