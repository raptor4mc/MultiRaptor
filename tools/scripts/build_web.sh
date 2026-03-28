#!/usr/bin/env bash
set -euo pipefail

if ! command -v emcmake >/dev/null 2>&1; then
  echo "Error: emcmake was not found in PATH."
  echo "Install/activate Emscripten SDK first, then re-run this script."
  exit 1
fi

BUILD_DIR="${1:-build-web}"

emcmake cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DMAGPHOS_BUILD_WASM=ON \
  -DMAGPHOS_BUILD_CLI=OFF \
  -DMAGPHOS_BUILD_REPL=OFF

cmake --build "${BUILD_DIR}"

if [[ ! -s web/magphos_wasm.js ]]; then
  echo "Error: web/magphos_wasm.js was not generated."
  exit 1
fi

if [[ ! -s web/magphos_wasm.wasm ]]; then
  echo "Error: web/magphos_wasm.wasm was not generated."
  exit 1
fi

echo "Web artifacts ready:"
echo "  - web/magphos_wasm.js"
echo "  - web/magphos_wasm.wasm"
