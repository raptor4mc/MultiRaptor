#!/usr/bin/env bash
set -euo pipefail

BASELINE_FILE="${1:-perf/baseline.json}"

if [[ ! -f "$BASELINE_FILE" ]]; then
  echo "Missing baseline file: $BASELINE_FILE" >&2
  exit 1
fi

MAX_MS="$(python3 - <<'PY' "$BASELINE_FILE"
import json, sys
print(json.load(open(sys.argv[1]))["runtime_tests_ms_max"])
PY
)"

START_NS="$(date +%s%N)"
bash tools/scripts/run_tests.sh >/tmp/magphos_perf_tests.log 2>&1
END_NS="$(date +%s%N)"

ELAPSED_MS="$(( (END_NS - START_NS) / 1000000 ))"
echo "runtime_tests wall time: ${ELAPSED_MS} ms (max ${MAX_MS} ms)"

if (( ELAPSED_MS > MAX_MS )); then
  echo "Performance regression detected: ${ELAPSED_MS}ms > ${MAX_MS}ms" >&2
  exit 1
fi
