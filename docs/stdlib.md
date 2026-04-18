# MagPhos Standard Library Reference

This file documents **runtime builtins** and the current `Lib/` helper modules.

## Runtime builtins (implemented in C++)

### Core
- `len(value)`
- `type(value)`
- `toString(value)`
- `random()`
- `time()`

### Math
- `sin(number)`
- `cos(number)`
- `sqrt(number)`
- `abs(number)`
- `tan(number)`
- `asin(number)`
- `acos(number)`
- `atan(number)`
- `log(number)` (base-10)
- `ln(number)` (natural log)
- `exp(number)`
- `pow(base, exponent)`
- `floor(number)`
- `ceil(number)`
- `round(number)`

### Strings
- `split(string, delimiter)`
- `replace(string, from, to)`
- `substring(string, start, length?)`
- `join(arrayOfStrings, delimiter)`
- `regexMatch(string, pattern)`
- `regexReplace(string, pattern, replacement)`

### Arrays
- `push(array, value)`
- `pop(array)`
- `map(array, builtinName)`
- `filter(array, mode, threshold?)`

### File I/O (native)
- `readFile(path)`
- `writeFile(path, content)`
- `appendFile(path, content)`
- `fileExists(path)`
- aliases: `read(path)`, `write(path, content)`, `append(path, content)`

### Objects / classes (interop-friendly)
- `objectCreate()`
- `objectSet(object, key, value)` (returns updated object)
- `objectGet(object, key)` (returns value or `null`)
- `classCreate(name)` (returns class descriptor value)

### Networking
- `httpGet(url)`
- `tcpConnect(ipv4Host, port)`
- `socketSend(socket, data)`
- `socketRecv(socket, maxBytes)`
- `socketClose(socket)`

### Interoperability
- `env(name)` (read OS environment variable)
- `exec(command)` (execute shell command and capture stdout)

### Concurrency & threading
- `threadSpawn(delayMs, value?)`
- `threadAwait(threadHandle)`
- `mutexCreate()`, `mutexLock(mutex)`, `mutexUnlock(mutex)`
- `semaphoreCreate(initialCount)`, `semaphoreAcquire(sem)`, `semaphoreRelease(sem)`
- `channelCreate()`, `channelSend(channel, value)`, `channelRecv(channel)`

### Game/Graphics (preview)
- `canvasCreate(width, height)`
- `inputIsKeyDown(key)`
- `timerNowMs()`
- `spriteLoad(path)`
- `spriteDraw(canvas, sprite, x, y)`
- `audioPlay(path)`

## `Lib/` helper modules

`Lib/` provides lightweight wrapper modules that can be imported with dotted names (for example `import io.file`).

### Important compatibility notes
- The wrappers are implemented in MagPhos source (`.mp`) and must call real runtime builtins.
- `print` is a **statement** in MagPhos syntax, not a builtin function.
- `Lib/json.mp` is intentionally minimal right now:
  - `encode(value)` uses `toString(value)`
  - `decode(text)` returns the input text unchanged
- `Lib/io/stream.mp` currently treats handles as file paths.
- `Lib/core/panic.mp` currently prints a panic message and returns `null`.

## Source of truth
- Runtime builtin registration: `src/runtime/stdlib/stdlib.cpp`
- Semantic builtin allow-list: `src/compiler/semantic/analyzer.cpp`
- Wrapper modules: `Lib/**/*.mp`
