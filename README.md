# atomic-kv

[![CI](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)
[![Tests](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg?event=push)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)
[![clang-tidy](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml/badge.svg?query=clang-tidy)](https://github.com/danielmg123/atomic-kv/actions/workflows/ci.yml)

## Quick Start

```bash
# clone & bootstrap
git clone https://github.com/danielmg123/atomic-kv.git
cd atomic-kv
./scripts/bootstrap.sh 

# build & test
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure