#include <iostream>
#include <format>
#include <cmath>

#include "CacheImplementations/LRUCache.h"
#include "CacheImplementations/LFUCache.h"
#include "CacheImplementations/LRFUCache.h"
#include "CacheImplementations/adaptive_replacement_cache.h"
#include "CacheImplementations/LIRSCache.h"
#include "CacheImplementations/DLIRSCache.h"
#include "CacheImplementations/OPTCache.h"
#include "Datasets/dataset_parser.h"
#include "metrics.h"

const double kMaxCacheFraction = 0.005;
const size_t kMaxCacheSize = 5'000;
const double kSizeMultiplier = 1.5;
const size_t kTestsCount = 30;

std::unordered_set<std::string> correct_cache_names = {
    "lru", "lfu", "lrfu", "arc", "lirs", "dlirs", "opt",
};

std::vector<size_t> GenerateCacheSizes(size_t min_size,
                                       size_t max_size,
                                       size_t num_points) {
  std::vector<size_t> sizes;
  if (num_points == 0) return sizes;
  if (num_points == 1) {
    sizes.push_back(min_size);
    return sizes;
  }

  const double p = 1.7;
  for (size_t i = 0; i < num_points; ++i) {
    double t = double(i) / double(num_points - 1);

    double scaled_t = std::pow(t, p);

    size_t current_size =
        min_size + std::round(scaled_t * double(max_size - min_size));

    if (sizes.empty() || current_size > sizes.back()) {
      sizes.push_back(current_size);
    } else {
      sizes.push_back(sizes.back() + 1);
    }
  }

  for (unsigned long& size : sizes) {
    if (size > max_size) {
      size = max_size;
    }
  }
  sizes.erase(std::unique(sizes.begin(), sizes.end()), sizes.end());

  return sizes;
}

int main(int argc, char* argv[]) {
  if (argc <= 2) {
    throw std::runtime_error{"Too few arguments"};
  }

  std::string cache_name = argv[1];
  std::string dataset_path = argv[2];

  if (!correct_cache_names.contains(cache_name)) {
    throw std::runtime_error{std::format("Unknown cache name: {}", cache_name)};
  }

  auto dataset = ParseDataset(dataset_path);
  size_t dataset_size = GetDatasetSize(dataset);
  size_t max_size = std::ceil(dataset_size * kMaxCacheFraction);
  auto testing_sizes = GenerateCacheSizes(4, max_size, 20);

  for (auto size : testing_sizes) {
    double cache_fraction = double(size) / double(dataset_size);
    // if (cache_fraction > kMaxCacheFraction) {
    //   break;
    // }

    auto cache_factory =
        [size, &cache_name, &any_dataset = dataset]<typename T>() -> AnyCache {
          if (cache_name == "lru") {
            return std::shared_ptr<LRUCache<T>>(new LRUCache<T>(size));
          } else if (cache_name == "lfu") {
            return std::shared_ptr<LFUCache<T>>(new LFUCache<T>(size));
          } else if (cache_name == "lrfu") {
            return std::shared_ptr<LRFUCache<T>>(new LRFUCache<T>(size));
          } else if (cache_name == "arc") {
            return std::shared_ptr<ARCache<T>>(new ARCache<T>(size));
          } else if (cache_name == "lirs") {
            return std::shared_ptr<LIRSCache<T>>(new LIRSCache<T>(size));
          } else if (cache_name == "dlirs") {
            return std::shared_ptr<DLIRSCache<T>>(new DLIRSCache<T>(size));
          } else if (cache_name == "opt") {
            try {
              auto& dataset = std::get<Dataset<T>>(any_dataset);
              return std::shared_ptr<OPTCache<T>>(new OPTCache<T>(size, dataset.keys.begin(), dataset.keys.end()));
            } catch (const std::bad_variant_access&) {
              throw std::runtime_error{"Mismatched types: variant does not contain the expected Dataset<T>"};
            }
          }
          throw std::runtime_error{
              std::format("Unexpected cache name {}", cache_name)};
        };

    auto m = MeasureCache(dataset, cache_factory);

    auto accuracy = m.GetAccuracy();
    std::cout << size << "\t" << cache_fraction << "\t" << accuracy << "\n";

    // if (std::abs(accuracy - 1) < 0.000001) {
    //   break;
    // }
  }
  return 0;
}
