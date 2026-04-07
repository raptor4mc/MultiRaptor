# MagPhos Syntax Reference

## Lexical basics
- Identifiers: `name`, `snake_case`, `_internal`
- Numbers: `1`, `42`, `3.14`
- Strings: `"hello"`
- Statement terminators: newline or `;`

## Declarations and assignment
```mp
var x = 10
const y = x + 2
x = y + 1
```

## Functions
```mp
fn add(a, b) {
  return a + b
}

result = add(2, 3)
```

Optional/default + variadic parameters:
```mp
fn greet(name = "friend", ...rest) {
  return name
}
```

## Control-style statements currently parsed
- `print <expr>`
- `return <expr>`
- block statements `{ ... }`
- `if <expr> { ... } else { ... }`
- `while <expr> { ... }`
- `for (<init>; <condition>; <increment>) { ... }`
- `when <expr> { ... }`
- `loop <countExpr> { ... }`
- `repeat while <expr> { ... }`
- `try { ... } catch { ... }`
- `switch <expr> { case <expr> { ... } default { ... } }`
- `match <expr> { case <expr> { ... } default { ... } }`
- `namespace <name> { ... }`
- `set <name> = <expr>`
- `ask <promptExpr> -> <name>`
- comments: `# ...` and `// ...`

Safety notes:
- `name = expr` and `set name = expr` require `name` to already exist.
- `ask "..." -> name` requires `name` to already exist.
- `return` is only valid inside `fn` blocks.
- visibility markers `public`/`private` are accepted before declarations.

## Modules
```mp
import math
import game.engine
use "utils.mp"
```

## Expression precedence
1. Grouping and call: `(expr)`, `name(args...)`
2. Unary: `-x`
3. Multiplicative: `*`, `/`
4. Additive: `+`, `-`

## Terminators
Statements can end with:
- newline
- semicolon (`;`)


## Experimental syntax (opt-in)
The following constructs are proposed and tracked in `docs/novel_language_experiments.md`.
They are **not** part of the stable grammar contract unless an explicit experimental flag enables them.

```mp
# 1) Time-travel variable snapshots
timeline hp = 100
print hp@0

# 2) Causality guard
because score from "trusted.sensor.v2" {
  grant_reward(score)
} else {
  audit("untrusted")
}

# 3) Counterfactual branch
whatif base {
  spawn_boss("Hydra")
} compare {
  print diff(world)
} commit_if (diff(world).fun_score > 0.8)

# 4) Diagnostics tone preference
mood diagnostics = "mentor"

# 5) Ambiguity-preserving pattern matching
match all parse("1/2/03") {
  case date_us(d)  => print(d)
  case date_eu(d)  => print(d)
}

# 6) Compile-time capability negotiation
negotiate runtime {
  require "deterministic_rng"
  prefer "simd128"
  fallback "portable_scalar"
}
```
