# MagPhos Roadmap

## Near-term
- Stabilize parser grammar and AST compatibility.
- Publish and enforce SemVer language/version stability policy (minor-line syntax/AST freeze, deprecation windows, migration guides).
- Formalize error model + observability contract (machine-readable codes, trace IDs, structured logs, stack traces).
- Expand runtime execution coverage beyond arithmetic/function calls.
- Improve module loading (caching + cycle detection).
- Improve stdlib API consistency and docs examples.

## Mid-term
- Bytecode or IR layer for optimized execution.
- Real package registry + lock file format (checksums + reproducible installs).
- WASM distribution workflow and web playground unification.

## Long-term
- Language server support.
- Package ecosystem, version constraints, dependency solver, and enterprise-grade deterministic resolution guarantees.
- Multi-backend runtime targets.
