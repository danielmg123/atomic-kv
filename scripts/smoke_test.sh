#!/usr/bin/env bash
set -euo pipefail

# Determine project root and binary directory
script_dir=$(cd "$(dirname "$0")" && pwd)
project_root=$(dirname "$script_dir")

if [ -n "${BUILD_DIR-}" ]; then
  # BUILD_DIR is relative to project root
  dir="$project_root/$BUILD_DIR"
else
  dir="$project_root/build"
fi

# Start the server
"$dir/kv_server" 12345 &
pid=$!
trap "kill $pid" EXIT

# Wait up to 5s for port 12345 to open
echo "Waiting for server to bind on port 12345…"
for i in {1..50}; do
  if nc -z 127.0.0.1 12345; then
    break
  fi
  sleep 0.1
done

echo "Server is up—running smoke tests"

# SET
out=$("$dir/kv_cli" SET testkey hello)
if [[ "$out" != "+OK" ]]; then
  echo "SET failed: $out"
  exit 1
fi
echo "  SET OK"

# GET
out=$("$dir/kv_cli" GET testkey)
if [[ "$out" != "+VALUE hello" ]]; then
  echo "GET failed: $out"
  exit 1
fi
echo "  GET OK"

# DEL
out=$("$dir/kv_cli" DEL testkey)
if [[ "$out" != "+OK" ]]; then
  echo "DEL failed: $out"
  exit 1
fi
echo "  DEL OK"

echo "Smoke test passed."