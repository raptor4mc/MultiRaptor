# Error model & observability contract

This document defines the machine-readable error/observability surface for MagPhos CLI and runtime-facing tools.

## Stability guarantee

- Error contract fields are versioned and treated as API surface.
- Existing JSON keys remain backward-compatible across patch/minor releases.
- New fields may be added additively.
- Existing `errorCode` values are never repurposed for a different meaning.

## Standard domains and codes

### Domains

- `cli`
- `parser`
- `semantic`
- `runtime`

### Code families

- CLI: `CLI_IO_ERROR`, `CLI_UNKNOWN_COMMAND`
- Parser: `PARSER_PARSE_ERROR`
- Semantic: `SEMANTIC_ANALYSIS_ERROR`
- Runtime envelope: `RUNTIME_EXECUTION_ERROR`, `RUNTIME_UNHANDLED_EXCEPTION`
- Runtime detailed code: `runtimeErrorCode` (ex: `RUNTIME_FAILURE`, `RUNTIME_TYPE_ERROR`)

## JSON response contract

For `magphos_cli --json` responses, the error/observability keys are:

- `errorDomain` (string, empty on success)
- `errorCode` (string, empty on success)
- `runtimeErrorCode` (runtime detail code; empty unless runtime failure)
- `traceId` (stable per CLI invocation, included in logs)
- `stackTrace` (array of frames for runtime failures)
- `logs` (array of structured log entries)

Each `logs[]` entry:

- `level` (`info`/`warn`/`error`)
- `code` (machine-readable event code)
- `message` (human-readable text)
- `traceId` (same trace id as top-level response)

## Backward-compatibility tests

`tests/cli_tests.cpp` includes assertions that lock contract behavior for:

- parser failures emitting `errorDomain=parser` and `errorCode=PARSER_PARSE_ERROR`,
- runtime failures emitting `errorDomain=runtime`, `errorCode=RUNTIME_EXECUTION_ERROR`,
- presence of `traceId`, `logs`, and runtime `stackTrace` fields.

These tests are part of the API compatibility safety net for tooling and enterprise integrations.
