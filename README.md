# MagPhos

MagPhos is the language name.

- File extension / short name: **`.mp`**
- Meaning of `mp`: **MagnesiumPhosphorus**

MagPhos is built to be **download-first** (local compiler on your machine), with a **web playground bonus** for quick testing and Chromebook-friendly access.

## What is in this repository

- Native compiler source: `src/MagPhos_compiler.cpp`
- Web playground: `web/playground.html`
- Example MagPhos program: `main.mp`

## How to download and make it work (current project)

### 1) Download the project

```bash
git clone <your-repo-url>
cd MultiRaptor
```

Or download ZIP from GitHub and extract it.

### 2) Build the compiler

#### Recommended (CMake)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Compiler output:

- Linux/macOS: `build/MagPhos_compiler`
- Windows (multi-config): `build/Release/MagPhos_compiler.exe`

#### One-file build (no CMake)

```bash
g++ -std=c++17 -O2 -o MagPhos_compiler src/MagPhos_compiler.cpp
```

### 3) Compile and run your `.mp` code

```bash
./build/MagPhos_compiler main.mp output.js
node output.js
```

If you used one-file build:

```bash
./MagPhos_compiler main.mp output.js
node output.js
```

### 4) Optional local install

```bash
cmake --install build --prefix .local
```

Binary path after install:

- `.local/bin/MagPhos_compiler`

## Browser playground (bonus mode)

Open `web/playground.html` directly in a browser, write MagPhos code, then click **Compile** and **Run**.

This is support mode (especially useful on Chromebook). The primary workflow remains the downloadable local compiler.

## Syntax overview

```txt
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
```

Supported statements:

- `var`, `const`, `set`
- `print`, `ask "question" -> name`
- `if`, `when`, `else`
- `loop`, `repeat while`
- `fn`, `return`
- `stop`, `next`
- comments: `/!`, `//! ... !/`, `#`, `//`
