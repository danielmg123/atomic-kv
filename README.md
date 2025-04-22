# atomic-kv

[![CI](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)

A high-performance, lock-free key-value store written in modern C++23.

## Quick Start

```bash
# Clone the repo
git clone https://github.com/danielmg123/atomic-kv.git
cd atomic-kv

# Bootstrap dependencies (via vcpkg)
./scripts/bootstrap.sh

# Configure and build (dev mode)
cmake --preset dev
cmake --build --preset dev

# Run tests (unit + integration)
ctest --preset dev --output-on-failure
```

## Demo

Once the build succeeds, you can start the server and issue commands via the CLI:

```bash
# Start the server on port 12345 (in the background)
build/kv_server 12345 &

# In another shell, set a key
build/kv_cli SET foo bar
# → +OK

# Retrieve the value
build/kv_cli GET foo
# → +VALUE bar

# Delete the key
build/kv_cli DEL foo
# → +OK
```

## Performance Benchmarks

```bash
# 1) Run all benchmarks (0.5 s min, 5 reps), output CSVs:
./scripts/run_bench.sh

# 2) Plot scaling trends:
python3 scripts/plot_bench.py
```