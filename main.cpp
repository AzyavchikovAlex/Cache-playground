#include <bits/stdc++.h>

template<typename K>
class Cache {
 public:
  virtual void Insert(const K& key) = 0;
  virtual void Touch(const K& key) = 0;
  [[nodiscard]] virtual bool IsPresent(const K& key) const = 0;
};

struct CacheMetrics {
  int requests_count{0};
  int cache_misses{0};
};

template<typename K>
struct Dataset {
  std::vector<K> keys;
};

template<typename K>
CacheMetrics MeasureCache(const Dataset<K>& dataset,
                          std::shared_ptr<Cache<K>> cache) {
  CacheMetrics metrics;
  for (const auto& key : dataset.keys) {
    metrics.requests_count += 1;
    if (cache->IsPresent(key)) {
      cache->Touch(key);
    } else {
      metrics.cache_misses += 1;
      cache->Insert(key);
    }
  }

  return metrics;
}

template<typename K>
class LRUCache : public Cache<K>{
 private:
  size_t size_;
  int64_t timestamp_{0};
  std::unordered_map<K, int64_t> keys_timestamps; // key -> time
  std::set<std::pair<int64_t, K>> queue_; // timestamp, key

  void EraseInternal(const K& key) {
    auto time_it = keys_timestamps.find(key);
    assert(time_it != keys_timestamps.end());
    queue_.erase({time_it->second, key});
    keys_timestamps.erase(time_it);
  }

 public:
  explicit LRUCache(size_t size) : size_(size) {
  }

  void Insert(const K& key) override {
    if (IsPresent(key)) {
      return;
    }
    if (queue_.size() >= size_) {
      auto [timestamp, key_to_erase] = *queue_.begin();
      EraseInternal(key_to_erase);
    }

    int64_t key_timestamp = timestamp_++;
    keys_timestamps[key] = key_timestamp;
    queue_.insert({key_timestamp, key});
  }

  void Touch(const K& key) override {
    auto time_it = keys_timestamps.find(key);
    assert(time_it != keys_timestamps.end());
    queue_.erase({time_it->second, key});
    time_it->second = timestamp_++;
    queue_.insert({time_it->second, key});
  }

  bool IsPresent(const K& key) const override {
    return keys_timestamps.contains(key);
  }
};

std::string GetMetrics(const CacheMetrics& m) {
  return "Accuracy %= " + std::to_string((m.requests_count - m.cache_misses) / double(m.requests_count));
}

int main() {
  Dataset<int> data {
      .keys = {
          // 1. Прогрев кэша (интенсивный повтор трех элементов)
          1, 2, 3, 1, 2, 3, 1, 2, 3, 1,

          // 2. Периодический паттерн (циклический опрос)
          1, 2, 3, 4, 1, 2, 3, 4, 1, 2,

          // 3. Сканирование / Всплеск «холодных» данных (заставит кэш вытеснить старое)
          10, 11, 12, 13, 14, 15, 16, 17, 18, 19,

          // 4. Паттерн 80/20 (частый возврат к популярным ключам 1 и 2)
          1, 2, 1, 5, 1, 2, 1, 6, 1, 2,
          1, 7, 1, 2, 1, 8, 1, 2, 1, 9,

          // 5. Временная локальность (пачки одинаковых запросов подряд)
          5, 5, 5, 5, 2, 2, 2, 2, 3, 3,
          3, 3, 4, 4, 4, 4, 1, 1, 1, 1,

          // 6. Случайный шум с редкими повторами
          8, 14, 2, 22, 1, 30, 3, 41, 4, 52,

          // 7. Проверка LRU/LFU (чередование старых популярных и новых редких)
          1, 99, 2, 98, 3, 97, 1, 96, 2, 95,

          // 8. Финальный стресс-тест на попадание (проверяем, что выжило в кэше)
          1, 2, 3, 4, 5, 1, 2, 3, 4, 5
      },
  };
  std::shared_ptr<Cache<int>> cache = std::make_shared<LRUCache<int>>(15000);

  auto m = MeasureCache(data, cache);
  std::cout << GetMetrics(m) << "\n";
  return 0;
}
