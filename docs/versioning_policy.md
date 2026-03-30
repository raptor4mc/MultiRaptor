# Language/version stability policy

MagPhos follows **Semantic Versioning (SemVer)** for language/tooling releases:

- **MAJOR (`X.0.0`)**: may include breaking grammar/AST/runtime API changes.
- **MINOR (`0.Y.0` or `1.Y.0`)**: additive features only; no silent breakage of stabilized syntax/AST in that minor line.
- **PATCH (`X.Y.Z`)**: bug fixes, diagnostics, docs, and non-breaking tooling improvements.

## Compatibility guarantees

### 1) Syntax + AST freeze per minor line

For a given minor line (example: `1.4.x`):

- Accepted syntax is frozen for all patch releases in that line.
- AST shape/field semantics are frozen for compiler/analysis consumers in that line.
- Existing valid source must continue to parse and analyze the same way (except for bug-fix diagnostics).
- Frozen signatures are tracked in `contracts/language_contract.json` and enforced in CI.

If a syntax or AST change would break this guarantee, it must be deferred to the next MAJOR (or guarded behind an explicit opt-in flag).

### 2) Runtime/library compatibility

- Public CLI flags and documented stdlib contracts are stable across patch releases.
- Behavioral fixes are allowed in patch releases when correcting clearly incorrect behavior.
- Any intentionally breaking behavior requires deprecation + migration notes first.

## Deprecation policy

Deprecations must include:

1. **Announcement release** (feature marked deprecated in docs/changelog).
2. **Grace window**: minimum **2 minor releases** (or **90 days**, whichever is longer) before removal in stable `1.x`.
3. **Actionable warning**: diagnostics should identify deprecated syntax/API and recommended replacement.
4. **Migration guide**: concrete before/after examples and bulk-update guidance.

For `0.x` pre-1.0 lines, MagPhos still aims to honor the same window whenever practical, but reserves the right to shorten windows when critical architecture changes are required.

## Migration guide requirements

Any release with deprecations or removals must publish/update a migration section with:

- impacted syntax/API list,
- exact replacement forms,
- compatibility notes for parser/AST consumers,
- examples for common project patterns.

## Enforcement checklist (release gate)

Before publishing:

- [ ] SemVer bump type matches change scope.
- [ ] Grammar/AST diff reviewed against minor-line freeze.
- [ ] Deprecation windows validated.
- [ ] Migration guide updated (if required).
- [ ] Changelog entry includes compatibility impact.
