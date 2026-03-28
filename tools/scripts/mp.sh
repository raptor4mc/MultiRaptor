#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'USAGE'
MagPhos package/runtime helper

Usage:
  mp.sh install <package>
  mp.sh run <program.mp>
  mp.sh repl
USAGE
}

if [[ $# -lt 1 ]]; then
  usage
  exit 1
fi

cmd="$1"
arg="${2:-}"

case "$cmd" in
  install)
    pkg_dir=".mp_packages/$arg"
    mkdir -p "$pkg_dir"
    cat > "$pkg_dir/PACKAGE.txt" <<PKG
name=$arg
installed_at=$(date -u +%Y-%m-%dT%H:%M:%SZ)
source=local-registry-placeholder
PKG
    echo "Installed package '$arg' into $pkg_dir"
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
