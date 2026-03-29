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
- `tan(number)`
- `asin(number)`
- `acos(number)`
- `atan(number)`
- `log(number)` (base-10)
- `ln(number)` (natural log)
- `exp(number)`
- `pow(base, exponent)`
- `floor(number)`, `ceil(number)`, `round(number)`

## Strings
- `split(string, delimiter)`
- `replace(string, from, to)`
- `substring(string, start, length?)`
- `join(arrayOfStrings, delimiter)`
- `regexMatch(string, pattern)`
- `regexReplace(string, pattern, replacement)`

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
- simple aliases: `read(path)`, `write(path, content)`, `append(path, content)`

## Objects / classes (interop-friendly)
- `objectCreate()`
- `objectSet(object, key, value)` (returns updated object)
- `objectGet(object, key)` (returns value or `null`)
- `classCreate(name)` (returns class descriptor value)

## Networking
- `httpGet(url)` (requires `curl` availability in runtime environment)
- `tcpConnect(ipv4Host, port)`
- `socketSend(socket, data)`
- `socketRecv(socket, maxBytes)`
- `socketClose(socket)`

## Interoperability
- `env(name)` (read OS environment variable)
- `exec(command)` (execute shell command and capture stdout)

## Concurrency & threading
- `threadSpawn(delayMs, value?)`
- `threadAwait(threadHandle)` (async/await-style wait primitive)
- `mutexCreate()`, `mutexLock(mutex)`, `mutexUnlock(mutex)`
- `semaphoreCreate(initialCount)`, `semaphoreAcquire(sem)`, `semaphoreRelease(sem)`
- `channelCreate()`, `channelSend(channel, value)`, `channelRecv(channel)`

## Notes
Implemented in `src/runtime/stdlib/stdlib.cpp` and surfaced via `runtime::StandardLibrary`.


## Game/Graphics (preview)
- `canvasCreate(width, height)`
- `inputIsKeyDown(key)`
- `timerNowMs()`
- `spriteLoad(path)`
- `spriteDraw(canvas, sprite, x, y)`
- `audioPlay(path)`
