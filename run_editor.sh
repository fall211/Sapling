#!/bin/bash
set -e

ASSETS_PATH="${1:-$(dirname "$0")/Examples/Demo3D/Assets}"
ASSETS_PATH="$(cd "$ASSETS_PATH" && pwd)"

cd "$(dirname "$0")/Editor"

cmake -B build/Debug -DCMAKE_BUILD_TYPE=Debug .

cmake --build build/Debug

./build/Debug/SaplingEditor "$ASSETS_PATH"
