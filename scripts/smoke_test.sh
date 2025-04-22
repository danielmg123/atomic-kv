#!/usr/bin/env bash
set -euo pipefail

# Use provided build dir or default to ../build
dir=${BUILD_DIR:-$(dirname "$0")/../build}

${dir}/kv_server 12345 &
pid=$!
trap "kill $pid" EXIT

# Wait for the server to bind to port 12345 (up to 5s)
echo "Waiting for server to bind on port 12345…"
for i in {1..50}; do
  if nc -z 127.0.0.1 12345; then
    break
  fi
  sleep 0.1
done
echo "Server is up—running smoke tests"

out=$(${dir}/kv_cli SET testkey hello)
if [[ "$out" != "+OK" ]]; then
  echo "SET failed: $out"
  exit 1
fi

out=$(${dir}/kv_cli GET testkey)
if [[ "$out" != "+VALUE hello" ]]; then
  echo "GET failed: $out"
  exit 1
fi

out=$(${dir}/kv_cli DEL testkey)
if [[ "$out" != "+OK" ]]; then
  echo "DEL failed: $out"
  exit 1
fi

echo "Smoke test passed."