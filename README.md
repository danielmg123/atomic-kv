# atomic-kv

[![CI](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)

**atomic-kv** is a lock-free key-value store written in C++23, built to learn concurrent systems programming. `put` and `get` are lock-free; `erase` and safe memory reclamation are still in progress. See [Status](#status) for what does and doesn't work yet.

---

## Author

**Daniel Morales**  
[github.com/danielmg123](https://github.com/danielmg123)

---

## Features

- **Lock-Free Hash Map**  
  Built with atomic primitives, the core `LockFreeHashMap` supports lock-free `put` and `get` operations with no locks or mutexes.

- **C++23**  
  Designed using modern C++ practices and standard libraries like `std::optional` and `std::atomic`.

- **Multi-Threaded Benchmarks**  
  Multi-threaded benchmarks using [Google Benchmark](https://github.com/google/benchmark), with a Python plotting script.

- **Client/Server Architecture**  
  Provides a TCP server and command-line client (CLI) that communicate using a simple protocol (`GET`, `SET`, `DEL`) over sockets, powered by ASIO.

- **CI/CD and Static Analysis**  
  GitHub Actions runs a full CI pipeline with:
  - a `clang-format --dry-run --Werror` formatting check
  - `clang-tidy` linting
  - Build and test with CMake presets
  - Doxygen API doc generation, pushed to the `gh-pages` branch

  Separately, `.pre-commit-config.yaml` defines local `clang-format` and `clang-tidy` git hooks, which run once installed with `pre-commit install`.

- **Cross-Platform Setup via CMake Presets**  
  Configurations for:
  - `dev` (macOS ARM64)
  - `ci` (Ubuntu CI/CD)
  - `perf` (release build without sanitizers; pinned to the `arm64-osx` vcpkg triplet, so macOS only)

- **API Documentation**  
  Doxygen based documentation is generated automatically via CI from the `Doxyfile`. The Doxygen run writes its output to `docs/api/html/`, which is gitignored and not present in a fresh clone.

---

## Design

`LockFreeHashMap` ([`include/lock_free_hash_map.hpp`](include/lock_free_hash_map.hpp)) is a fixed-size hash table with separate chaining:

- **Buckets.** An array of `std::atomic<Node*>` bucket heads, each fronting a singly-linked chain of nodes. The bucket count is a template parameter fixed at construction, 2^20 by default. The table never resizes.
- **`put`.** Allocates a new node and prepends it to its bucket's chain with a compare-and-swap loop on the bucket head. If another thread changes the head first, the CAS fails, the new node is re-pointed at the current head, and the loop retries.
- **`get`.** Walks the chain with acquire loads of the bucket head and each node's `next` pointer, reading each node's key and value with plain reads. It writes no shared memory, so concurrent readers never block and never invalidate each other's cached lines.

The ASIO TCP server and CLI described under [Features](#features) sit on top of the map: the server keeps its data in a `LockFreeHashMap<std::string, std::string>` and handles `GET`, `SET` and `DEL` requests on a thread pool.

---

## Status

`put` and `get` are lock-free and work. The following are not yet correct:

- **`erase` is not concurrency-safe.** The retire list is an unsynchronized `std::vector`, so concurrent `erase` calls race and abort. The server's `DEL` command calls `erase`, so concurrent `DEL` requests hit this too.
- **Reclamation is not true epoch-based reclamation.** Readers never announce an epoch, so a node can be freed while another thread is reading it, and `global_epoch_` counts retirements rather than marking reclamation periods.
- **`erase` can resurrect an old value.** `put` prepends rather than replaces, so duplicate keys accumulate on a chain, and `erase` unlinks only the first match.
- **Unlinking has no deletion marking**, so concurrent erases of neighboring nodes can corrupt a chain.
- **`put` never frees**, so write-heavy use grows memory without bound.

Correct epoch-based reclamation here would require:

- readers publishing an epoch on entry;
- per-thread retire lists, with each node stamped with the epoch at retirement;
- reclamation restricted to nodes retired at least two epochs back;
- Harris-style marked pointers on unlink, so a node is logically deleted before it is physically removed.

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
- **Core Libraries**: ASIO, Google Benchmark, GoogleTest
- **Build Tools**: CMake, Ninja, vcpkg
- **CI/CD**: GitHub Actions, clang-format, clang-tidy
- **Docs**: Doxygen
