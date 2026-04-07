# MagPhos Roadmap

## Near-term
- Stabilize parser grammar and AST compatibility.
- Enforce per-release grammar/AST freeze via language contract signatures in CI.
- Publish and enforce SemVer language/version stability policy (minor-line syntax/AST freeze, deprecation windows, migration guides).
- Formalize error model + observability contract (machine-readable codes, trace IDs, structured logs, stack traces).
- Enforce CI quality gates: sanitizers, fuzzing, coverage thresholds, performance regression checks, and cross-platform matrix.
- Establish LTS lifecycle policy, security patch SLAs, and CVE/responsible-disclosure process.
- Publish production hardening guide and breaking-change playbook with migration tooling.
- Expand runtime execution coverage beyond arithmetic/function calls.
- Lock down runtime determinism + edge-case error semantics with contract tests.
- Improve module loading (deterministic cache behavior + explicit cycle detection).
- Improve stdlib API consistency and docs examples.


## Experimental track (language innovation)
- Prototype novel syntax and semantics in `docs/novel_language_experiments.md` (timeline variables, causality guards, counterfactual blocks, ambiguity-preserving pattern matching, and compile-time negotiation).
- Keep innovation features gated behind explicit experimental flags until deterministic runtime semantics and migration guarantees are proven.

## Mid-term
- Bytecode or IR layer for optimized execution.
- Real package registry + lock file format (checksums + reproducible installs).
- WASM distribution workflow and web playground unification.

## Long-term
- Language server support.
- Package ecosystem, version constraints, dependency solver, and enterprise-grade deterministic resolution guarantees.
- Multi-backend runtime targets.
