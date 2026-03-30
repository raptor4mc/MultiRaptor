#!/usr/bin/env bash
set -euo pipefail

echo "[policy] formatting check"
bash tools/scripts/format_code.sh --check

echo "[policy] deterministic dependency lockfile"
python3 tools/scripts/check_lockfile_determinism.py

echo "[policy] language contract freeze"
python3 tools/scripts/check_language_contract.py

echo "[policy] warnings as errors build"
cmake -S . -B build-policy -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Werror"
cmake --build build-policy -j2

echo "[policy] API compatibility checks"
bash tools/scripts/run_tests.sh
