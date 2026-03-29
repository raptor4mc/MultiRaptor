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
- `set <name> = <expr>`
- `ask <promptExpr> -> <name>`
- comments: `# ...` and `// ...`

Safety notes:
- `name = expr` and `set name = expr` require `name` to already exist.
- `ask "..." -> name` requires `name` to already exist.
- `return` is only valid inside `fn` blocks.

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
