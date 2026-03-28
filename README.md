# MoPr

MoPr (MultiRaptor) is a tiny experimental programming language with a **desktop-first workflow** and a **web playground companion**.

## Project direction

The goal is:

1. **Download and code locally first** (main experience).
2. **Keep browser support as a bonus**, so Chromebook users (and anyone without local toolchain setup) can still code in MoPr.

That means MoPr is not "web-only". The browser is convenience; the native/compiler download is the primary path.

## What is in this repository

- A **native C++ compiler** (`src/MoPr_compiler.cpp`) that transpiles `.mp` files to JavaScript.
- A simple **browser playground** (`web/playground.html`) for instant experimentation.

## Language syntax (simple + readable)

```txt
# comments use #, //, or /!
//! starts a multi-line comment block (end with !/)
var x = 10
const name = "Raptor"
set x = x + 1

fn greet(person) {
  print "Hello " + person
}

when x > 5 {
  print "big"
} else {
  print "small"
}

repeat while x < 20 {
  set x = x + 2
}
```

Supported statements:

- `var name = expr` (create variable)
- `const name = expr` (constant)
- `set name = expr` (change variable value)
- `print expr`
- `ask "question" -> name` (read user input in browser)
- `if condition { ... }`
- `when condition { ... }` (alias for `if`)
- `else { ... }`
- `loop count { ... }` (counted loop)
- `repeat while condition { ... }` (while loop)
- `fn name(args) { ... }`
- `return expr`
- `stop` (`break`) and `next` (`continue`)
- Comments: `/! one line`, `//!` ... `!/`, plus `#` and `//`

## Build (download-first workflow)

### Option A: CMake (recommended)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Compiler binary:

- `build/MoPr_compiler` (Linux/macOS)
- `build/Release/MoPr_compiler.exe` (Windows, multi-config generators)

Optional local install:

```bash
cmake --install build --prefix .local
```

### Option B: single-file compile

```bash
g++ -std=c++17 -O2 -o MoPr_compiler src/MoPr_compiler.cpp
```

## Compile and run a MoPr program

```bash
./MoPr_compiler main.mp output.js
node output.js
```

## Browser playground (bonus)

Open `web/playground.html` in your browser and use **Compile** / **Run**.

## Notes on performance

- The compiler is a fast line-based parser + direct JS emitter.
- Runtime speed follows JavaScript engine performance.
- The compiler can later be compiled to WebAssembly to share one compiler core between native and browser environments.
