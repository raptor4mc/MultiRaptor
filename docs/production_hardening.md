# Production hardening guide

This guide provides baseline controls for running MagPhos in production.

## Runtime sandboxing

- Run workloads in isolated containers/VMs.
- Use read-only filesystem mounts where possible.
- Deny outbound network unless explicitly needed.
- Restrict process privileges (non-root user, seccomp/AppArmor profile).

## Stdlib allowlisting

High-risk stdlib calls (file/network/process) should be policy-gated:

- `readFile`, `writeFile`, `appendFile`
- `httpGet`, `tcpConnect`
- `exec`

Recommended: wrap CLI/runtime invocation with an allowlist policy and reject disallowed calls in production workloads.

## Dependency/module pinning

- Commit `mp.lock.json` and enforce deterministic lockfile checks in CI.
- Disallow floating dependency ranges for production deployments.
- Pin module sources to reviewed internal registry mirrors.

## Auditing and traceability

- Store CLI JSON output with `traceId`, `errorCode`, and `runtimeErrorCode`.
- Retain structured logs for incident timelines.
- Record build provenance (commit SHA, compiler/toolchain versions).

## Release gate checklist

- [ ] deterministic dependency checks passing
- [ ] sanitizer/fuzz/perf/coverage gates passing
- [ ] migration audit reviewed for release candidate
- [ ] security advisories reviewed against target deployment line
