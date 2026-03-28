# MagPhos Standard Library Reference

## Core
- `len(value)`
- `type(value)`
- `toString(value)`
- `random()`
- `time()`

## Math
- `sin(number)`
- `cos(number)`
- `sqrt(number)`
- `abs(number)`

## Strings
- `split(string, delimiter)`
- `replace(string, from, to)`
- `substring(string, start, length?)`

## Arrays
- `push(array, value)`
- `pop(array)`
- `map(array, builtinName)`
- `filter(array, mode, threshold?)`

## File I/O (native)
- `readFile(path)`
- `writeFile(path, content)`

## Notes
Implemented in `src/runtime/stdlib.cpp` and surfaced via `runtime::StandardLibrary`.
