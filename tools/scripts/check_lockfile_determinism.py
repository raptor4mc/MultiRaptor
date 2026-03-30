#!/usr/bin/env python3
import json
import pathlib
import re
import sys


LOCKFILE = pathlib.Path("mp.lock.json")
HEX64 = re.compile(r"^sha256-[0-9a-f]{64}$")
EXACT_VERSION = re.compile(r"^\d+\.\d+\.\d+(?:[-+][A-Za-z0-9.\-]+)?$")


def fail(message: str) -> None:
    print(f"[lockfile-check] ERROR: {message}")
    sys.exit(1)


def main() -> None:
    if not LOCKFILE.exists():
        fail("mp.lock.json is missing.")

    raw = LOCKFILE.read_text(encoding="utf-8")
    try:
        data = json.loads(raw)
    except json.JSONDecodeError as exc:
        fail(f"invalid JSON: {exc}")

    canonical = json.dumps(data, indent=2, sort_keys=True) + "\n"
    if raw != canonical:
        fail("lockfile is not canonical JSON. Reformat with sorted keys + 2-space indent.")

    packages = data.get("packages")
    if not isinstance(packages, list):
        fail("packages must be an array.")

    keys = [(pkg.get("name", ""), pkg.get("version", "")) for pkg in packages]
    if keys != sorted(keys):
        fail("packages must be sorted lexicographically by (name, version).")

    for pkg in packages:
        name = pkg.get("name")
        version = pkg.get("version")
        integrity = pkg.get("integrity", "")
        if not isinstance(name, str) or not name:
            fail("package entry missing non-empty name.")
        if not isinstance(version, str) or not EXACT_VERSION.match(version):
            fail(f"{name}: version must be exact SemVer pin, got {version!r}.")
        if not isinstance(integrity, str) or not HEX64.match(integrity):
            fail(f"{name}@{version}: integrity must match sha256-<64 hex>.")

        deps = pkg.get("dependencies", {})
        if not isinstance(deps, dict):
            fail(f"{name}@{version}: dependencies must be an object.")
        for dep_name, dep_version in deps.items():
            if not isinstance(dep_version, str) or not EXACT_VERSION.match(dep_version):
                fail(
                    f"{name}@{version}: dependency {dep_name!r} must be exact SemVer, got {dep_version!r}."
                )

    print("[lockfile-check] OK: deterministic lockfile validated.")


if __name__ == "__main__":
    main()
