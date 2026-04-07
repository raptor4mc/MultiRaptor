# Language contract freeze

MagPhos freezes grammar and AST compatibility per release line.

## Contract artifact

- Contract file: `contracts/language_contract.json`
- Guard script: `tools/scripts/check_language_contract.py`

The contract stores frozen signatures for:

- `docs/grammar.md` (grammar contract)
- `src/compiler/ast/nodes.h` (AST shape contract)

## CI behavior

CI runs the contract check and fails when grammar/AST signatures drift unexpectedly.

If a change is intentional:

1. Follow `docs/versioning_policy.md` and `docs/breaking_change_playbook.md`.
2. Update release notes/migration docs.
3. Refresh the contract during release prep with:

```bash
python3 tools/scripts/check_language_contract.py --update
```


## Experimental exclusions

Any syntax documented as experimental (for example `docs/novel_language_experiments.md` and the experimental appendix in `docs/grammar.md`) is excluded from contract freeze checks until promoted to stable.
