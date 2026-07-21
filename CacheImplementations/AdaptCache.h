#pragma once

#include <unordered_set>

#include "cache_interface.h"

template<typename K>
class AdaptCache : public NewCache<K> {
 private:
  struct PolicyInfo {
    std::shared_ptr<KeyReplacementPolicy<K>> policy;
    std::unordered_set<K> ghost_keys;
    int64_t hit_count{0};
    int64_t wins_count{0};
  };

  std::vector<PolicyInfo> policies_;
  std::unordered_set<K> data_;
  size_t max_cache_size_{1};
  const double kSwitchPeriodRatio{0.2};
  const size_t kMinSwitchPeriod{32};
  const double kSwitchThresholdRatio{0.05};
  const size_t kMinWinsCount{5};
  int64_t epoch_{0};
  size_t switch_period_{kMinSwitchPeriod};
  size_t main_policy_{0};

 private:
  void SetMainPolicyEvictCallback(size_t policy_index) {
    policies_[policy_index].policy->SetEvictCallback([this, policy_index](const K& key) -> void {
      auto& info = policies_[policy_index];
      if (!data_.contains(key)) {
        return;
      }
      if (data_.size() <= max_cache_size_) {
        policies_[policy_index].ghost_keys.insert(key);
        return;
      }

      K erasing_key = key;
      if (!info.ghost_keys.empty()) {
        erasing_key = *info.ghost_keys.begin();
        assert(erasing_key != key);
        info.ghost_keys.insert(key);
      }

      data_.erase(erasing_key);
      for (auto& another_info : policies_) {
        another_info.ghost_keys.erase(erasing_key);
      }
    });
  }

  void SetSecondaryPolicyEvictCallback(size_t policy_index) {
    policies_[policy_index].policy->SetEvictCallback([this, policy_index](const K& key) -> void {
      if (!data_.contains(key)) {
        return;
      }
      policies_[policy_index].ghost_keys.insert(key);
    });
  }

  void CompleteEpoch() {
    ++epoch_;

    if (epoch_ == switch_period_) {
      // collect stats
      int64_t max_hits = 0;
      size_t winner = main_policy_;
      for (size_t i = 0; i < policies_.size(); ++i) {
        auto& info = policies_[i];
        if (info.hit_count > max_hits) {
          max_hits = info.hit_count;
          winner = i;
        }
      }
      double win_ratio =
          (policies_[winner].hit_count - policies_[main_policy_].hit_count) /
              static_cast<double>(switch_period_);
      if (win_ratio >= kSwitchThresholdRatio) {
        policies_[winner].wins_count += 1;
        for (size_t i = 0; i < policies_.size(); ++i) {
          if (i != winner) {
            policies_[i].wins_count = 0;
          }
        }
      }
      if (winner != main_policy_ &&
          policies_[winner].wins_count >= kMinWinsCount) {
        std::cerr << "SWITCH\t" << winner << "\n";
        // policies switch
        SetSecondaryPolicyEvictCallback(main_policy_);
        SetMainPolicyEvictCallback(winner);
        main_policy_ = winner;
      }

      for (auto& info : policies_) {
        info.hit_count = 0;
      }
      epoch_ = 0;
    }
  }

 public:
  explicit AdaptCache(std::initializer_list<std::shared_ptr<KeyReplacementPolicy<
      K>>> policies,
                      size_t cache_size) :
      max_cache_size_(cache_size),
      switch_period_(std::max<size_t>(cache_size * kSwitchPeriodRatio,
                                      kMinSwitchPeriod)) {
    if (policies.begin() == policies.end()) {
      throw std::runtime_error(
          "no key replacement policy provided for AdaptCache");
    }
    if (cache_size == 0) {
      throw std::runtime_error("Zero size is invalid for AdaptCache");
    }
    for (auto it = policies.begin(); it != policies.end(); ++it) {
      policies_.push_back(PolicyInfo{
          .policy = std::move(*it),
      });
      policies_.back().policy->Resize(cache_size);
    }

    for (size_t i = 0; i < policies_.size(); ++i) {
      if (i == main_policy_) {
        SetMainPolicyEvictCallback(main_policy_);
      } else {
        SetSecondaryPolicyEvictCallback(i);
      }
    }
  }

  bool Get(const K& key) override {
    bool is_present = data_.contains(key);

    for (auto& policy_info : policies_) {
      policy_info.hit_count += policy_info.policy->Contains(key);
      if (is_present) {
        policy_info.ghost_keys.erase(key);
        policy_info.policy->Touch(key);
      }
    }
    CompleteEpoch();

    return is_present;
  }

  void Set(const K& key) override {
    bool is_present = data_.contains(key);

    // touch key
    data_.insert(key);
    for (auto& info : policies_) {
      info.ghost_keys.erase(key);
      info.policy->Touch(key);
    }
    assert(data_.contains(key) == true);

    // evict some key from cache
    if (data_.size() > max_cache_size_) {
      assert(data_.size() == 1 + max_cache_size_);
      auto& main_info = policies_[main_policy_];
      // try to evict from ghost keys
      assert(!main_info.ghost_keys.empty());
      K erasing_key = *main_info.ghost_keys.begin();
      main_info.ghost_keys.erase(main_info.ghost_keys.begin());
      assert(data_.contains(erasing_key));
      data_.erase(erasing_key);
      for (auto& info : policies_) {
        info.ghost_keys.erase(erasing_key);
      }
    }
  }
};