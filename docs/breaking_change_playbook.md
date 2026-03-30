# Breaking-change playbook

Use this process for any change that can break language/runtime/tooling compatibility.

## 1) Proposal and impact classification

- classify impact: syntax, AST, runtime behavior, CLI contract, stdlib
- list affected versions/channels (Current/LTS)
- identify migration strategy and tooling support needed

## 2) Deprecation phase

- introduce deprecation warnings and docs updates
- publish timeline in changelog + roadmap
- keep compatibility bridge for minimum deprecation window (see versioning policy)

## 3) Migration tooling

- update `tools/scripts/migration_audit.py` rules
- publish before/after examples and bulk migration patterns
- provide CI mode (`--strict`) to block non-migrated syntax in release branches

## 4) Release execution

- ship migration guide with release notes
- announce breaking changes with exact versions/dates
- validate API compatibility tests and observability contract tests

## 5) Post-release monitoring

- monitor error telemetry by `errorCode`
- track migration audit findings
- publish follow-up advisory for common migration failures
