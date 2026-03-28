# MagPhos Syntax Reference

## Lexical basics
- Identifiers: `name`, `snake_case`, `_internal`
- Numbers: `1`, `42`, `3.14`
- Strings: `"hello"`
- Comments:
  - line: `# comment`
  - line: `// comment`
  - block-start marker: `/!`

## Declarations and assignment
```mp
x = 10
y = x + 2
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
