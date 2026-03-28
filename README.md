# MagPhos

MagPhos is the language name.

- File extension / short name: **`.mp`**
- Meaning of `mp`: **MagnesiumPhosphorus**

MagPhos is built to be **download-first** (local compiler on your machine), with a **web playground bonus** for quick testing and Chromebook-friendly access.

## Download places (versioned)

> We will keep adding a **new row on every update** so people can download older/newer versions.

| Version | Date | Download |
|---|---|---|
| v0.1.2 | 2026-03-28 | https://github.com/<owner>/<repo>/archive/refs/heads/work.zip |
| v0.1.1 | 2026-03-28 | https://github.com/<owner>/<repo>/archive/refs/heads/main.zip |
| v0.1.0 | 2026-03-28 | https://github.com/<owner>/<repo>/releases |

Replace `<owner>/<repo>` with your real GitHub path.

## What is in this repository

- Native compiler source: `src/MagPhos_compiler.cpp`
- Core module files (lexer/parser/ast/runtime/interpreter/utils) are now wired into the build target.
- Web playground: `web/playground.html`
- Example MagPhos program: `main.mp`

## How to download and make it work (current project)

### 1) Download the project

```bash
git clone <your-repo-url>
cd MultiRaptor
```

Or download ZIP from the "Download places" table above.

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
g++ -std=c++17 -Iinclude -Isrc -O2 -o MagPhos_compiler src/MagPhos_compiler.cpp src/ast/nodes.cpp src/interpreter/interpreter.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/runtime/enviroment.cpp src/runtime/value.cpp src/utils/error.cpp src/utils/string_utils.cpp src/main.cpp
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
