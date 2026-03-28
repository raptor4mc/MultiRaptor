# MagPhos REPL Guide

MagPhos now ships a native REPL executable (`magphos_repl`) and script wrapper (`mp.sh repl`).

## Start REPL
```bash
./tools/scripts/mp.sh repl
```

Prompt:
```text
mp>
```

## Example session
```text
mp> var x = 10
ok
mp> x = x + 5
ok
mp> :quit
```

## Notes
- REPL currently executes one-line statements.
- Uses the same lexer/parser/runtime engine as normal execution.
