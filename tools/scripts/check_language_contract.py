#!/usr/bin/env python3
"""
Language contract guard:
- freezes grammar + AST signatures per release line
- fails CI when signatures drift unexpectedly
"""

from __future__ import annotations
import argparse
import hashlib
import json
import pathlib
import sys


CONTRACT_FILE = pathlib.Path("contracts/language_contract.json")
GRAMMAR_FILE = pathlib.Path("docs/grammar.md")
AST_FILE = pathlib.Path("src/compiler/ast/nodes.h")


def sha256(path: pathlib.Path) -> str:
    data = path.read_bytes()
    return hashlib.sha256(data).hexdigest()


def load_contract() -> dict:
    if not CONTRACT_FILE.exists():
        return {
            "releaseLine": "0.1.x",
            "semverPolicyDoc": "docs/versioning_policy.md",
            "breakingChangePolicyDoc": "docs/breaking_change_playbook.md",
            "frozen": {},
        }
    return json.loads(CONTRACT_FILE.read_text(encoding="utf-8"))


def current_signature() -> dict:
    return {
        "grammarSha256": sha256(GRAMMAR_FILE),
        "astSha256": sha256(AST_FILE),
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--update", action="store_true", help="Update frozen signature to current files.")
    args = parser.parse_args()

    contract = load_contract()
    signature = current_signature()

    if args.update:
        contract["frozen"] = signature
        CONTRACT_FILE.parent.mkdir(parents=True, exist_ok=True)
        CONTRACT_FILE.write_text(json.dumps(contract, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print(f"[language-contract] updated {CONTRACT_FILE}")
        return 0

    frozen = contract.get("frozen", {})
    if not frozen:
        print("[language-contract] no frozen signature present. Run with --update first.")
        return 1

    failures = []
    for key, value in signature.items():
        if frozen.get(key) != value:
            failures.append((key, frozen.get(key, "<missing>"), value))

    if failures:
        print("[language-contract] Contract drift detected:")
        for key, expected, actual in failures:
            print(f" - {key}: expected {expected}, got {actual}")
        print("If this is an intentional breaking change, follow SemVer/breaking-change playbook and refresh with --update in release prep.")
        return 1

    print("[language-contract] OK: grammar/AST signatures match frozen contract.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
