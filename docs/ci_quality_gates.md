# CI quality gates

MagPhos CI now enforces stability beyond basic pass/fail tests:

- **Cross-platform matrix**: Linux/macOS/Windows compile + test (`quality-matrix.yml`)
- **Sanitizers**: ASan+UBSan and TSan (`sanitizers.yml`)
- **Fuzzing**: lexer/parser fuzz smoke runs (`fuzz.yml`)
- **Performance regression check**: wall-time threshold against `perf/baseline.json` (`perf-regression.yml`)
- **Coverage threshold**: line coverage gate (`coverage.yml`)

## Scripts

- `tools/scripts/run_tests.sh` — unified compile+run for current test binaries.
- `tools/scripts/check_perf_regression.sh` — compares runtime against baseline.
- `tools/scripts/check_coverage.sh` — produces and enforces gcovr line coverage threshold.

These gates are intended to provide statistical confidence in runtime/grammar stability over time.
