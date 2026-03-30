#!/usr/bin/env bash
set -euo pipefail

MIN_LINE="${1:-65}"

rm -rf build-coverage
cmake -S . -B build-coverage -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="--coverage -O0 -g"
cmake --build build-coverage -j2

bash tools/scripts/run_tests.sh

python3 -m pip install --quiet gcovr
gcovr -r . --exclude-directories build-coverage --xml-pretty -o coverage.xml
gcovr -r . --exclude-directories build-coverage --fail-under-line "$MIN_LINE"
