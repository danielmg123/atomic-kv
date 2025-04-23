#!/usr/bin/env bash
set -euo pipefail

# If vcpkg isn't already cloned, fetch and bootstrap it
if [ ! -d "vcpkg" ]; then
  echo "Cloning vcpkg..."
  git clone https://github.com/microsoft/vcpkg.git vcpkg
  echo "Bootstrapping vcpkg..."
  (cd vcpkg && ./bootstrap-vcpkg.sh)
else
  echo "vcpkg already present. Skipping clone."
fi

# Notify user how to integrate vcpkg
cat <<EOF

Bootstrap complete! To use vcpkg in this project:
  cmake --preset dev
  cmake --build --preset dev
EOF