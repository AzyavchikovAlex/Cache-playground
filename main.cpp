#include <iostream>

#include "Datasets/dataset_parser.h"
#include "CacheImplementations/LRUCache.h"
#include "metrics.h"

int main() {
  auto dataset = ParseDataset("Datasets/1_synthetic/sample1.txt");

  for (size_t size = 1; size < 100; ++size) {
    auto cache_factory = [size]<typename T>() -> AnyCache {
      return std::shared_ptr<LRUCache<T>>(new LRUCache<T>(size));
    };

    auto m = MeasureCache(dataset, cache_factory);
    std::cout << size << "\t" << m.GetAccuracy() << "\n";
    if (std::abs(m.GetAccuracy() - 1) < 0.000001) {
      break;
    }
  }
  return 0;
}
