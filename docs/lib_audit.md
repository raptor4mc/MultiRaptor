# Lib, Code, and Documentation Audit

Date: 2026-04-18

## Scope
- Reviewed all `Lib/**/*.mp` modules for syntax/runtime-builtin compatibility.
- Reviewed runtime/semantic sources that define callable APIs:
  - `src/runtime/stdlib/stdlib.cpp`
  - `src/runtime/engine/errors.{h,cpp}`
  - `src/runtime/engine/module_system.{h,cpp}`
  - `src/compiler/semantic/analyzer.cpp`
- Audited **all markdown files** in the repository (42 total) for function and path consistency.

## What was verified
1. `Lib/` wrappers only call implemented MagPhos builtins (no `__core_*` calls remain).
2. Markdown function references written as ``name(...)`` resolve to real symbols (runtime builtins, runtime C++ APIs, or `Lib` functions).
3. Markdown path references are valid, except intentionally compressed notation such as brace-expanded docs in README.

## Findings
- No unresolved markdown function references remain after audit.
- `Lib` wrappers are aligned to available builtins.
- Remaining README path-style shorthand (for example `src/runtime/errors.{h,cpp}`) is intentional notation, not missing files.

## Notes on current `Lib` behavior
- `Lib/json.mp` is currently a minimal compatibility shim (`encode -> toString`, `decode -> identity`).
- `Lib/io/stream.mp` currently uses file-path semantics for handles.
- `Lib/core/panic.mp` currently prints a panic message and returns `null`.
