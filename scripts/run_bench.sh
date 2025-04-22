#!/usr/bin/env bash
set -euo pipefail

# 0) (Re)bootstrap vcpkg
if [ ! -f vcpkg/scripts/buildsystems/vcpkg.cmake ]; then
  echo "Bootstrapping vcpkg…"
  ./scripts/bootstrap.sh
fi

# 1) Configure & build in perf mode
cmake --preset perf
cmake --build --preset perf

# 2) Ensure results directory exists
mkdir -p benchmarks/results

# 3) Run benchmarks with stable settings and CSV output
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUT="benchmarks/results/bench_${TIMESTAMP}.csv"
echo "Running benchmarks -> $OUT"
build-perf/kv_bench \
  --benchmark_min_time=0.5s \
  --benchmark_repetitions=5 \
  --benchmark_report_aggregates_only=true \
  --benchmark_format=csv \
  > "$OUT"

echo "Benchmarks complete — CSVs in benchmarks/results/"