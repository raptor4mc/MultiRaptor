#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-fix}"

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format is required." >&2
  exit 1
fi

mapfile -t FILES < <(rg --files -g '*.cpp' -g '*.h' -g '*.hpp' -g '*.cc' -g '*.cxx')

if [[ "${#FILES[@]}" -eq 0 ]]; then
  echo "No C/C++ files found."
  exit 0
fi

if [[ "$MODE" == "--check" ]]; then
  clang-format --dry-run --Werror "${FILES[@]}"
  echo "Formatting check passed."
else
  clang-format -i "${FILES[@]}"
  echo "Formatted ${#FILES[@]} files."
fi
