# Runtime correctness contract

This contract defines expected runtime/module behavior stability.

## Runtime error semantics

- Error categories must remain stable (`NameError`, `TypeError`, `ArityError`, `ScopeError`, `RuntimeFailure`).
- `runtimeErrorCodeName(...)` string values are treated as machine-readable API.

## Determinism expectations

- Given identical source + environment, runtime output and error codes should be deterministic.
- Module loads are deterministic with explicit cache behavior:
  - first load performs locate → parse → compile/execute and resolves dependencies,
  - repeated load returns cached content,
  - `clearCache()` forces reload from disk.

## Module loading safety

- Recursive import/use chains are followed for dependency loading.
- Cycles must fail with explicit cyclic-import/use errors.
- Import resolution checks caller-local paths first, then stdlib search roots (`lib/` and `Lib/`) when available.
- Cache and cycle detection behavior is covered by runtime tests (`tests/runtime_tests.cpp`).

## Layering invariant

- Core runtime (C++) remains standalone and owns parser, VM/interpreter, memory model, builtins, and import system.
- Language-level standard modules in `lib/` (`Lib/` in this repository) are optional high-level extensions.
- Dependency direction is strictly `core <- lib`; core must not depend on language-level library modules.
