#include <benchmark/benchmark.h>

#include <memory>

#include "../include/lock_free_hash_map.hpp"

using kv::LockFreeHashMap;

//-----------------------------------------------------------------------------
// Single‑threaded insertion of N elements into a fresh map
//-----------------------------------------------------------------------------
static void BM_Put(benchmark::State& state) {
  auto N = state.range(0);
  for (auto _ : state) {
    // build on the heap to avoid huge stack frames
    auto map = std::make_unique<LockFreeHashMap<int, int>>();
    for (int i = 0; i < N; ++i) {
      map->put(i, i + 0);
    }
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Put)->Arg(10'000)->Arg(100'000);

//-----------------------------------------------------------------------------
// Single‑threaded lookup of N existing elements
//-----------------------------------------------------------------------------
static void BM_GetExisting(benchmark::State& state) {
  auto N = state.range(0);
  for (auto _ : state) {
    auto map = std::make_unique<LockFreeHashMap<int, int>>();
    for (int i = 0; i < N; ++i) {
      map->put(i, i + 0);
    }
    for (int i = 0; i < N; ++i) {
      auto v = map->get(i);
      benchmark::DoNotOptimize(v);
    }
  }
}
BENCHMARK(BM_GetExisting)->Arg(10'000)->Arg(100'000);

//-----------------------------------------------------------------------------
// Single‑threaded erase of N elements from a fresh map
//-----------------------------------------------------------------------------
static void BM_Erase(benchmark::State& state) {
  auto N = state.range(0);
  for (auto _ : state) {
    auto map = std::make_unique<LockFreeHashMap<int, int>>();
    for (int i = 0; i < N; ++i) {
      map->put(i, i + 0);
    }
    for (int i = 0; i < N; ++i) {
      map->erase(i);
    }
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_Erase)->Arg(10'000)->Arg(100'000);

//-----------------------------------------------------------------------------
// Multi‑threaded insertion: each thread adds its own range of keys
//-----------------------------------------------------------------------------
static void BM_ConcurrentPut(benchmark::State& state) {
  static LockFreeHashMap<int, int> map;
  auto per_thread = state.range(0);
  static thread_local int counter = 0;

  for (auto _ : state) {
    int tid = state.thread_index();
    int key = static_cast<int>(tid * per_thread + (counter++ % per_thread));
    map.put(key, key + 0);
    benchmark::ClobberMemory();
  }
}
BENCHMARK(BM_ConcurrentPut)->Arg(100'000)->Threads(1)->Threads(4)->Threads(8);

//-----------------------------------------------------------------------------
// Mixed workload: 50% puts, 50% gets on a pre‑populated map
//-----------------------------------------------------------------------------
static void BM_MixedWorkload(benchmark::State& state) {
  static LockFreeHashMap<int, int> map;
  static bool populated = false;
  auto total = state.range(0);

  // populate once before any timing
  if (!populated) {
    for (int i = 0; i < total; ++i) {
      map.put(i, i + 0);
    }
    populated = true;
  }

  static thread_local int counter = 0;
  for (auto _ : state) {
    int idx = static_cast<int>(counter++ % total);
    if ((counter & 1) == 0) {
      map.put(idx, idx + 0);
      benchmark::ClobberMemory();
    } else {
      auto v = map.get(idx);
      benchmark::DoNotOptimize(v);
    }
  }
}
BENCHMARK(BM_MixedWorkload)->Arg(100'000)->Threads(1)->Threads(4)->Threads(8);

// Provide main() via Google Benchmark:
BENCHMARK_MAIN();