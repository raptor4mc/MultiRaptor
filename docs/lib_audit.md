# Lib and Markdown Function Audit

Date: 2026-04-18

## Scope
- Reviewed all `Lib/**/*.mp` wrappers for runtime builtin compatibility.
- Cross-checked markdown function-call references (``name(...)``) against:
  - `Lib/**/*.mp` functions
  - runtime builtins in `src/runtime/stdlib/stdlib.cpp`
  - runtime/module API symbols in `src/runtime/engine/*.h/.cpp`
  - semantic builtin allow-list in `src/compiler/semantic/analyzer.cpp`

## Findings
- Replaced non-existent `__core_*` calls in `Lib/` with supported runtime builtins.
- Fixed `Lib/math/random.mp` to avoid unsupported `%` usage.
- Verified runtime-contract API names referenced in docs exist in C++ runtime code:
  - `runtimeErrorCodeName(...)`
  - `ModuleSystem::clearCache()`
- Non-runtime examples like `greet(...)`, `name(...)`, and `for(...)` in markdown are sample syntax, not stdlib/runtime API promises.

## Current status
- `Lib/` wrappers now parse with current MagPhos syntax and only use known runtime primitives.
- Intentionally simplified wrapper behavior is documented in `docs/stdlib.md`.
