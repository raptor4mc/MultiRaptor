#!/usr/bin/env bash
set -euo pipefail

EMSDK_VERSION="${MAGPHOS_EMSDK_VERSION:-3.1.72}"
EMSDK_DIR="${MAGPHOS_EMSDK_DIR:-.tools/emsdk}"
DOCKER_EMSDK_IMAGE="${MAGPHOS_DOCKER_EMSDK_IMAGE:-emscripten/emsdk:${EMSDK_VERSION}}"
USE_DOCKER_FALLBACK="${MAGPHOS_USE_DOCKER_FALLBACK:-1}"

BUILD_DIR="${1:-build-web}"

MIN_JS_BYTES="${MAGPHOS_MIN_WASM_JS_BYTES:-51200}"
MIN_WASM_BYTES="${MAGPHOS_MIN_WASM_BYTES:-102400}"
MIN_SINGLEFILE_JS_BYTES="${MAGPHOS_MIN_SINGLEFILE_JS_BYTES:-204800}"

ensure_emscripten() {
  if command -v emcmake >/dev/null 2>&1; then
    return 0
  fi

  echo "emcmake not found. Bootstrapping Emscripten SDK (${EMSDK_VERSION}) into ${EMSDK_DIR}..."
  mkdir -p "$(dirname "${EMSDK_DIR}")"

  if [[ ! -d "${EMSDK_DIR}" ]]; then
    if ! git clone https://github.com/emscripten-core/emsdk.git "${EMSDK_DIR}"; then
      echo "Warning: failed to clone emsdk from GitHub."
      return 1
    fi
  fi

  pushd "${EMSDK_DIR}" >/dev/null
  if ! ./emsdk install "${EMSDK_VERSION}"; then
    popd >/dev/null
    echo "Warning: failed to install emsdk ${EMSDK_VERSION}."
    return 1
  fi
  if ! ./emsdk activate "${EMSDK_VERSION}"; then
    popd >/dev/null
    echo "Warning: failed to activate emsdk ${EMSDK_VERSION}."
    return 1
  fi
  # shellcheck disable=SC1091
  source ./emsdk_env.sh
  popd >/dev/null

  if ! command -v emcmake >/dev/null 2>&1; then
    echo "Warning: emcmake is still unavailable after Emscripten bootstrap."
    return 1
  fi

  return 0
}

build_with_docker_emscripten() {
  if ! command -v docker >/dev/null 2>&1; then
    echo "Error: docker is not available for fallback web build."
    return 1
  fi

  local repo_root
  repo_root="$(pwd)"

  echo "Building web artifacts with Docker image ${DOCKER_EMSDK_IMAGE}..."
  docker run --rm \
    -u "$(id -u):$(id -g)" \
    -v "${repo_root}:/src" \
    -w /src \
    "${DOCKER_EMSDK_IMAGE}" \
    bash -lc 'emcmake cmake -S . -B "'"${BUILD_DIR}"'" -DCMAKE_BUILD_TYPE=Release -DMAGPHOS_BUILD_WASM=ON -DMAGPHOS_BUILD_CLI=OFF -DMAGPHOS_BUILD_REPL=OFF && cmake --build "'"${BUILD_DIR}"'"'
}

if ensure_emscripten; then
  emcmake cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DMAGPHOS_BUILD_WASM=ON \
    -DMAGPHOS_BUILD_CLI=OFF \
    -DMAGPHOS_BUILD_REPL=OFF

  cmake --build "${BUILD_DIR}"
elif [[ "${USE_DOCKER_FALLBACK}" == "1" ]]; then
  build_with_docker_emscripten
else
  echo "Error: Emscripten is unavailable and Docker fallback is disabled."
  echo "Hint: install emcmake, or set MAGPHOS_USE_DOCKER_FALLBACK=1 and install Docker."
  exit 1
fi

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

assert_min_bytes() {
  local path="$1"
  local minimum="$2"
  local size
  size=$(wc -c < "${path}")
  if (( size < minimum )); then
    echo "Error: ${path} is unexpectedly small (${size} bytes, expected at least ${minimum})."
    echo "Hint: this usually means a fallback/placeholder artifact was generated or checked in."
    exit 1
  fi
}

require_nonempty_file web/magphos_wasm.js
require_nonempty_file web/magphos_wasm.wasm.64
require_nonempty_file web/magphos_wasm_singlefile.js

assert_not_fallback_loader web/magphos_wasm.js
assert_not_fallback_loader web/magphos_wasm_singlefile.js
assert_wasm_binary_header web/magphos_wasm.wasm.64
assert_min_bytes web/magphos_wasm.js "${MIN_JS_BYTES}"
assert_min_bytes web/magphos_wasm.wasm.64 "${MIN_WASM_BYTES}"
assert_min_bytes web/magphos_wasm_singlefile.js "${MIN_SINGLEFILE_JS_BYTES}"

echo "Web artifacts ready (strict validation passed):"
echo "  - web/magphos_wasm.js ($(wc -c < web/magphos_wasm.js) bytes)"
echo "  - web/magphos_wasm.wasm.64 ($(wc -c < web/magphos_wasm.wasm.64) bytes)"
echo "  - web/magphos_wasm_singlefile.js ($(wc -c < web/magphos_wasm_singlefile.js) bytes)"
