#pragma once

#include <unordered_set>
#include <exception>

#include "Datasets/dataset.h"

struct CacheMetrics {
  int requests_count{0};
  int cache_misses{0};
  int unique_keys_count{0};

  [[nodiscard]] double GetAccuracy() const {
    double not_unique_requests_count = requests_count - unique_keys_count;
    double not_unique_cache_misses_count = cache_misses - unique_keys_count;
    if (not_unique_requests_count == 0.0) {
      return 1.0;
    }
    return (not_unique_requests_count - not_unique_cache_misses_count)
        / not_unique_requests_count;
  }
};

template<typename CacheFactory>
CacheMetrics MeasureCache(const AnyDataset& any_dataset,
                          CacheFactory& create_cache) {
  CacheMetrics metrics;
  std::visit([&create_cache, &metrics](const auto& dataset) -> void {
    using T = typename decltype(dataset.keys)::value_type;

    auto any_cache = create_cache.template operator()<T>();
    std::visit([&dataset, &metrics](const auto& cache) -> void {
      using CacheKeyType = typename std::decay_t<decltype(cache)>::element_type::KeyType;

      if constexpr (std::is_same_v<T, CacheKeyType>) {
        std::unordered_set<CacheKeyType> perfect_cache;
        for (const auto& key : dataset.keys) {
          metrics.requests_count += 1;
          if (!perfect_cache.contains(key)) {
            metrics.unique_keys_count += 1;
            perfect_cache.insert(key);
          }

          if (cache->Contains(key)) {
            cache->Touch(key);
          } else {
            metrics.cache_misses += 1;
            cache->Insert(key);
          }
        }
      } else {
        throw std::runtime_error(
            "Runtime type mismatch between dataset and cache");
      }
    }, any_cache);
  }, any_dataset);

  return metrics;
}