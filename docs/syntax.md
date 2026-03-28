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


> Note: `set`, `ask`, `when`, `loop`, `repeat while`, and comment syntax are not tokenized/parsed yet.
