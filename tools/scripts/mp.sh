#!/usr/bin/env bash
set -euo pipefail

LOCKFILE="mp.lock.json"
PKG_ROOT=".mp_packages"

usage() {
  cat <<'USAGE'
MagPhos package/runtime helper

Usage:
  mp.sh init [project_name]
  mp.sh install <package[@version]>
  mp.sh update <package[@version]>
  mp.sh verify
  mp.sh list
  mp.sh run <program.mp>
  mp.sh repl

Notes:
- package specs default to version '0.1.0' when omitted
- installs are deterministic and recorded in mp.lock.json
- verify checks installed package integrity against lockfile checksums
USAGE
}

sha256_file() {
  local file="$1"
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$file" | awk '{print $1}'
  elif command -v shasum >/dev/null 2>&1; then
    shasum -a 256 "$file" | awk '{print $1}'
  else
    echo "Missing sha256 tool (sha256sum or shasum required)." >&2
    exit 2
  fi
}

utc_now() {
  date -u +%Y-%m-%dT%H:%M:%SZ
}

ensure_lockfile() {
  python3 - <<'PY'
import json
from pathlib import Path

path = Path("mp.lock.json")
if path.exists():
    data = json.loads(path.read_text())
else:
    data = {}

if not isinstance(data, dict):
    data = {}

data.setdefault("schemaVersion", 1)
data.setdefault("generatedAt", "1970-01-01T00:00:00Z")
data.setdefault("resolver", {
    "algorithm": "deterministic-graph-v1",
    "registry": "local://magphos"
})
data.setdefault("packages", [])

if not isinstance(data.get("packages"), list):
    data["packages"] = []

# Canonical ordering
ordered = {
    "generatedAt": data["generatedAt"],
    "packages": data["packages"],
    "resolver": data["resolver"],
    "schemaVersion": data["schemaVersion"],
}
path.write_text(json.dumps(ordered, indent=2, sort_keys=False) + "\n")
PY
}

parse_spec() {
  local spec="$1"
  if [[ "$spec" == *"@"* ]]; then
    pkg_name="${spec%@*}"
    pkg_version="${spec#*@}"
  else
    pkg_name="$spec"
    pkg_version="0.1.0"
  fi

  if [[ -z "$pkg_name" || -z "$pkg_version" ]]; then
    echo "Invalid package spec: $spec" >&2
    exit 2
  fi
}

upsert_lock_package() {
  local name="$1"
  local version="$2"
  local integrity="$3"
  local source="$4"

  python3 - "$name" "$version" "$integrity" "$source" <<'PY'
import json
import sys
from datetime import datetime, timezone
from pathlib import Path

name, version, integrity, source = sys.argv[1:5]
path = Path("mp.lock.json")
if not path.exists():
    raise SystemExit("mp.lock.json missing")

data = json.loads(path.read_text())
pkgs = data.get("packages", [])

entry = {
    "dependencies": {},
    "integrity": integrity,
    "name": name,
    "source": source,
    "version": version,
}

updated = False
for i, pkg in enumerate(pkgs):
    if pkg.get("name") == name:
        pkgs[i] = entry
        updated = True
        break
if not updated:
    pkgs.append(entry)

pkgs.sort(key=lambda p: (p.get("name", ""), p.get("version", "")))
data["packages"] = pkgs
data["generatedAt"] = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")

ordered = {
    "generatedAt": data["generatedAt"],
    "packages": data["packages"],
    "resolver": data.get("resolver", {"algorithm": "deterministic-graph-v1", "registry": "local://magphos"}),
    "schemaVersion": data.get("schemaVersion", 1),
}
path.write_text(json.dumps(ordered, indent=2, sort_keys=False) + "\n")
PY
}

install_package() {
  local spec="$1"
  parse_spec "$spec"

  mkdir -p "$PKG_ROOT/$pkg_name"
  local manifest="$PKG_ROOT/$pkg_name/PACKAGE.txt"

  cat > "$manifest" <<PKG
name=$pkg_name
version=$pkg_version
installed_at=$(utc_now)
source=local-registry-placeholder
PKG

  local digest
  digest="$(sha256_file "$manifest")"

  ensure_lockfile
  upsert_lock_package "$pkg_name" "$pkg_version" "sha256-$digest" "local://registry/$pkg_name"

  echo "Installed $pkg_name@$pkg_version"
  echo "Integrity: sha256-$digest"
  echo "Lockfile updated: $LOCKFILE"
}

verify_lockfile_integrity() {
  ensure_lockfile
  python3 - <<'PY'
import json
import hashlib
from pathlib import Path
import sys

lock = Path("mp.lock.json")
data = json.loads(lock.read_text())
packages = data.get("packages", [])

errors = []
for pkg in packages:
    name = pkg.get("name", "")
    expected = pkg.get("integrity", "")
    manifest = Path(".mp_packages") / name / "PACKAGE.txt"

    if not manifest.exists():
        errors.append(f"{name}: missing installed manifest at {manifest}")
        continue

    digest = hashlib.sha256(manifest.read_bytes()).hexdigest()
    actual = f"sha256-{digest}"
    if expected != actual:
        errors.append(f"{name}: integrity mismatch (expected {expected}, got {actual})")

if errors:
    print("Integrity check failed:", file=sys.stderr)
    for err in errors:
        print(f"- {err}", file=sys.stderr)
    sys.exit(3)

print(f"Integrity check passed for {len(packages)} package(s).")
PY
}

init_project() {
  local name="${1:-magphos-project}"
  if [[ ! -f "main.mp" ]]; then
    cat > main.mp <<'MP'
#! New MagPhos project
print "hello from MagPhos"
MP
  fi

  if [[ ! -f "MagPhos.toml" ]]; then
    cat > MagPhos.toml <<CFG
name = "$name"
version = "0.1.0"
entry = "main.mp"
CFG
  fi

  mkdir -p "$PKG_ROOT"
  ensure_lockfile
  echo "Initialized project '$name' with main.mp, MagPhos.toml, and $LOCKFILE"
}

list_packages() {
  ensure_lockfile
  python3 - <<'PY'
import json
from pathlib import Path

path = Path("mp.lock.json")
data = json.loads(path.read_text())
packages = data.get("packages", [])

if not packages:
    print("No packages installed.")
else:
    for pkg in packages:
        print(f"{pkg.get('name')}@{pkg.get('version')} {pkg.get('integrity')}")
PY
}

if [[ $# -lt 1 ]]; then
  usage
  exit 1
fi

cmd="$1"
arg="${2:-}"

case "$cmd" in
  init)
    init_project "$arg"
    ;;
  install)
    if [[ -z "$arg" ]]; then
      echo "Usage: mp.sh install <package[@version]>" >&2
      exit 1
    fi
    install_package "$arg"
    ;;
  update)
    if [[ -z "$arg" ]]; then
      echo "Usage: mp.sh update <package[@version]>" >&2
      exit 1
    fi
    install_package "$arg"
    ;;
  verify)
    verify_lockfile_integrity
    ;;
  list)
    list_packages
    ;;
  run)
    if [[ ! -f "$arg" ]]; then
      echo "Program not found: $arg" >&2
      exit 2
    fi

    if [[ ! -x build/MagPhos_compiler ]]; then
      echo "Compiler missing, building first..."
      cmake -S . -B build
      cmake --build build
    fi

    out_js="/tmp/magphos_run_$$.js"
    ./build/MagPhos_compiler "$arg" "$out_js"

    if command -v node >/dev/null 2>&1; then
      node "$out_js"
    else
      echo "Compiled output written to $out_js (node not found)."
    fi
    ;;
  repl)
    if [[ ! -x build/magphos_repl ]]; then
      echo "REPL missing, building first..."
      cmake -S . -B build
      cmake --build build
    fi
    ./build/magphos_repl
    ;;
  *)
    usage
    exit 1
    ;;
esac
