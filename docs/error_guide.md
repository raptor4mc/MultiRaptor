# MagPhos Error Guide

## Parse errors
Parse errors include:
- line + column
- source snippet
- caret marker (`^`)
- message
- hint

Example:
```text
Error at line 4, column 8:
  x =
      ^
Expected expression.
Hint: Did you forget a value, identifier, or parentheses?
```

## Runtime errors
Runtime uses `RuntimeErrorCode` categories:
- `NameError`
- `TypeError`
- `ArityError`
- `ScopeError`
- `RuntimeFailure`

These are raised by the runtime engine/environment for undefined names,
wrong argument counts, bad operator types, and invalid runtime operations.
