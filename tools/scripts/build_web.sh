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

require_nonempty_file() {
  local path="$1"
  if [[ ! -s "${path}" ]]; then
    echo "Error: ${path} was not generated."
    exit 1
  fi
}

assert_not_fallback_loader() {
  local path="$1"
  if grep -Fq "Repo bundled fallback loader (WASM artifact not built)." "${path}"; then
    echo "Error: ${path} still contains the bundled fallback loader."
    echo "Hint: build_web.sh must produce real Emscripten output, not fallback placeholders."
    exit 1
  fi
}

assert_wasm_binary_header() {
  local path="$1"
  local header
  header=$(od -An -tx1 -N 4 "${path}" | tr -d " \n")
  if [[ "${header}" != "0061736d" ]]; then
    echo "Error: ${path} does not start with the wasm binary magic bytes (00 61 73 6d)."
    echo "Hint: this usually means a placeholder or base64 text file was written instead of raw wasm bytes."
    exit 1
  fi
}

require_nonempty_file web/magphos_wasm.js
require_nonempty_file web/magphos_wasm.wasm
require_nonempty_file web/magphos_wasm_singlefile.js

assert_not_fallback_loader web/magphos_wasm.js
assert_not_fallback_loader web/magphos_wasm_singlefile.js
assert_wasm_binary_header web/magphos_wasm.wasm

echo "Web artifacts ready:"
echo "  - web/magphos_wasm.js ($(wc -c < web/magphos_wasm.js) bytes)"
echo "  - web/magphos_wasm.wasm ($(wc -c < web/magphos_wasm.wasm) bytes)"
echo "  - web/magphos_wasm_singlefile.js ($(wc -c < web/magphos_wasm_singlefile.js) bytes)"
