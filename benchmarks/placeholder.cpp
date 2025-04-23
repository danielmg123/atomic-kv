#include <benchmark/benchmark.h>
static void BM_NoOp(benchmark::State& s) {
  for (auto _ : s)
    ;
}
BENCHMARK(BM_NoOp);
BENCHMARK_MAIN();