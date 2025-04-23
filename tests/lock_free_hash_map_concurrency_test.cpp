#include <gtest/gtest.h>

#include <thread>
#include <vector>

#include "../include/lock_free_hash_map.hpp"

using kv::LockFreeHashMap;

TEST(LockFreeHashMapConcurrency, ParallelInsertGet) {
  // Use static map so it lives in static storage
  static LockFreeHashMap<int, int, 1 << 20> map;

  const int num_threads = 8;
  const int ops_per_thread = 100000;

  // Launch concurrent inserters
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int t = 0; t < num_threads; ++t) {
    threads.emplace_back([t]() {
      int base = t * ops_per_thread;
      for (int i = 0; i < ops_per_thread; ++i) {
        map.put(base + i, base + i);
      }
    });
  }
  for (auto& th : threads) th.join();

  // Verify all entries
  for (int t = 0; t < num_threads; ++t) {
    int base = t * ops_per_thread;
    for (int i = 0; i < ops_per_thread; ++i) {
      auto v = map.get(base + i);
      EXPECT_TRUE(v.has_value());
      if (v) EXPECT_EQ(v.value(), base + i);
    }
  }
}