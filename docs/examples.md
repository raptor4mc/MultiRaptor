# MagPhos Examples Guide

## Hello world (`examples/hello_world.mp`)
Shows minimal expression printing.

## Loops (`examples/loops.mp`)
Demonstrates repeated statements with numeric loop count syntax used by the transpiler path.

## Functions (`examples/functions.mp`)
Shows `fn`, parameter passing, and call expressions.

## Recommended workflow
1. Read examples in this folder.
2. Parse-check them with `./tools/scripts/run_tests.sh` (includes example tests).
3. Run with `./tools/scripts/mp.sh run <example.mp>`.


## Experimental concept walkthrough
See `docs/novel_language_experiments.md` for non-stable examples that intentionally explore new language territory.
These snippets are design references and are not expected to pass the current parser test suite.
