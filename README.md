# atomic-kv

[![CI](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)

**atomic-kv** is a high performance, lock-free key-value store written in C++23. It features a concurrent hash map with epoch based memory reclamation and aims to provide a foundation for exploring systems programming, benchmarking, and scalable design.

---

## Author

**Daniel Morales**  
[github.com/danielmg123](https://github.com/danielmg123)

---

## Features

- **Lock-Free Hash Map**  
  Built with atomic primitives and epoch based garbage collection, the core `LockFreeHashMap` supports `put`, `get`, and `erase` operations with no locks or mutexes.

- **C++23**  
  Designed using C++ practices, leveraging smart pointers, thread-local storage, and standard libraries like `std::optional` and `std::atomic`.

- **Multi-Threaded Benchmarks**  
  Includes detailed performance benchmarking using [Google Benchmark](https://github.com/google/benchmark), with scaling plots generated via Python (Pandas + Matplotlib).

- **Client/Server Architecture**  
  Provides a TCP server and command-line client (CLI) that communicate using a simple protocol (`GET`, `SET`, `DEL`) over sockets, powered by ASIO.

- **CI/CD and Static Analysis**  
  GitHub Actions runs a full CI pipeline with:
  - `clang-format` (enforced via pre-commit hooks)
  - `clang-tidy` linting
  - Build and test with CMake presets
  - Auto-deployment of API docs with Doxygen to GitHub Pages

- **Cross-Platform Setup via CMake Presets**  
  Configurations for:
  - `dev` (macOS ARM64)
  - `ci` (Ubuntu CI/CD)
  - `perf` (release build without sanitizers)

- **API Documentation**  
  Doxygen based documentation is generated and deployed automatically via CI. See the `Doxyfile` and `docs/api/` directory.

---

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

---

## Tech Stack

- **Languages**: C++23, Python
- **Core Libraries**: ASIO, FMT, spdlog, Google Benchmark, GoogleTest
- **Build Tools**: CMake, Ninja, vcpkg
- **CI/CD**: GitHub Actions, clang-format, clang-tidy
- **Docs**: Doxygen + GitHub Pages