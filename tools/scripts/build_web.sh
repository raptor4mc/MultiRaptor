#!/usr/bin/env bash
set -euo pipefail

EMSDK_VERSION="${MAGPHOS_EMSDK_VERSION:-3.1.72}"
EMSDK_DIR="${MAGPHOS_EMSDK_DIR:-.tools/emsdk}"

BUILD_DIR="${1:-build-web}"

ensure_emscripten() {
  if command -v emcmake >/dev/null 2>&1; then
    return 0
  fi

  echo "emcmake not found. Bootstrapping Emscripten SDK (${EMSDK_VERSION}) into ${EMSDK_DIR}..."
  mkdir -p "$(dirname "${EMSDK_DIR}")"

  if [[ ! -d "${EMSDK_DIR}" ]]; then
    git clone https://github.com/emscripten-core/emsdk.git "${EMSDK_DIR}"
  fi

  pushd "${EMSDK_DIR}" >/dev/null
  ./emsdk install "${EMSDK_VERSION}"
  ./emsdk activate "${EMSDK_VERSION}"
  # shellcheck disable=SC1091
  source ./emsdk_env.sh
  popd >/dev/null

  if ! command -v emcmake >/dev/null 2>&1; then
    echo "Error: emcmake is still unavailable after Emscripten bootstrap."
    exit 1
  fi
}

ensure_emscripten

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

if [[ ! -s web/magphos_wasm_singlefile.js ]]; then
  echo "Error: web/magphos_wasm_singlefile.js was not generated."
  exit 1
fi

cat > web/magphos_artifacts.json <<'EOF'
{
  "loaders": [
    "./magphos_wasm_singlefile.js",
    "./magphos_wasm.js"
  ],
  "wasm": [
    "./magphos_wasm.wasm"
  ]
}
EOF

echo "Web artifacts ready:"
echo "  - web/magphos_wasm.js ($(wc -c < web/magphos_wasm.js) bytes)"
echo "  - web/magphos_wasm.wasm ($(wc -c < web/magphos_wasm.wasm) bytes)"
echo "  - web/magphos_wasm_singlefile.js ($(wc -c < web/magphos_wasm_singlefile.js) bytes)"
echo "  - web/magphos_artifacts.json ($(wc -c < web/magphos_artifacts.json) bytes)"
