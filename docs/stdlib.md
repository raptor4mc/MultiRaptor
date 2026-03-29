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
- `appendFile(path, content)`
- `fileExists(path)`

## Objects / classes (interop-friendly)
- `objectCreate()`
- `objectSet(object, key, value)` (returns updated object)
- `objectGet(object, key)` (returns value or `null`)
- `classCreate(name)` (returns class descriptor value)

## Networking
- `httpGet(url)` (requires `curl` availability in runtime environment)

## Interoperability
- `env(name)` (read OS environment variable)
- `exec(command)` (execute shell command and capture stdout)

## Notes
Implemented in `src/runtime/stdlib.cpp` and surfaced via `runtime::StandardLibrary`.


## Game/Graphics (preview)
- `canvasCreate(width, height)`
- `inputIsKeyDown(key)`
- `timerNowMs()`
- `spriteLoad(path)`
- `spriteDraw(canvas, sprite, x, y)`
- `audioPlay(path)`
