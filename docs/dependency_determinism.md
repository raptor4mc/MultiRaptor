# Deterministic dependency system policy

MagPhos dependency resolution is designed for **reproducible installs** across machines and CI.

## Goals

- Same `mp.lock.json` + same platform rules => same resolved dependency graph.
- Every locked artifact is content-addressed with checksums.
- CI can verify lockfile determinism and fail fast on drift.

## Registry model (phase 1)

- A package registry entry must provide:
  - immutable package name + version,
  - artifact URL (or blob id),
  - SHA-256 digest for the exact distributable artifact.
- Re-publishing changed bytes under the same name/version is forbidden.

## Lockfile format (`mp.lock.json`)

Top-level keys:

- `schemaVersion` (integer)
- `generatedAt` (ISO-8601 UTC string)
- `resolver` (object)
- `packages` (array, deterministic order)

Each package entry:

- `name` (string)
- `version` (exact version, no ranges)
- `source` (registry URL/id)
- `integrity` (`sha256-<64 hex chars>`)
- `dependencies` (object map of child package -> exact version)

## Determinism rules

1. `packages` must be lexicographically sorted by `(name, version)`.
2. All dependency edges in lockfiles must be **exact** versions (no `^`, `~`, `>=`).
3. Every package entry must include `integrity` checksum.
4. Lockfile must be canonical JSON formatting (stable key order + formatting).

## CI enforcement

MagPhos CI runs `tools/scripts/check_lockfile_determinism.py` to validate:

- canonical formatting,
- sorted package ordering,
- exact-version dependency pins,
- integrity checksum format.

Any violation fails CI to block non-reproducible dependency state from landing.


## `mp.sh` workflow (current)

Deterministic package workflow commands:

- `./tools/scripts/mp.sh init <project_name>`
- `./tools/scripts/mp.sh install <name[@version]>`
- `./tools/scripts/mp.sh update <name[@version]>`
- `./tools/scripts/mp.sh list`
- `./tools/scripts/mp.sh verify`

Behavior guarantees:

- install/update records package entries in `mp.lock.json` sorted by `(name, version)`
- lock entries include `integrity` as `sha256-...`
- package manifests are stored at `.mp_packages/<name>/PACKAGE.txt`
- `verify` recomputes checksums from installed manifests and fails fast on mismatch
