#!/usr/bin/env python3
"""
Lightweight migration audit helper.
Scans .mp files for known-breaking syntax markers and emits guidance.
"""

from __future__ import annotations
import argparse
import pathlib
import sys


RULES = [
    ("<!DOCTYPE MAGPHOS>", "Prefer explicit <magphos> root for HTMLXD files."),
    ("set ", "Verify semantic scope behavior after parser/runtime upgrades."),
]


def iter_mp_files(root: pathlib.Path):
    for path in root.rglob("*.mp"):
        if ".git" in path.parts:
            continue
        yield path


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=".", help="Repo root to scan.")
    parser.add_argument("--strict", action="store_true", help="Fail if migration findings are present.")
    args = parser.parse_args()

    findings = []
    root = pathlib.Path(args.root)
    for file_path in iter_mp_files(root):
        text = file_path.read_text(encoding="utf-8", errors="ignore")
        for needle, msg in RULES:
            if needle in text:
                findings.append((file_path, needle, msg))

    if findings:
        print("[migration-audit] Findings:")
        for file_path, needle, msg in findings:
            print(f" - {file_path}: found '{needle}' -> {msg}")
    else:
        print("[migration-audit] No known migration markers found.")

    return 1 if findings and args.strict else 0


if __name__ == "__main__":
    sys.exit(main())
