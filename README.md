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
- Web studio playground: `web/playground.html`
- Example MagPhos program: `main.mp`

## How to download and make it work (current project)

### 1) Download the project

```bash
git clone <your-repo-url>
cd MagPhos
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

Open `web/playground.html` directly in a browser to use **MagPhos Web Studio**.

Web Studio includes a Chromebook-friendly coding workspace with:

- file explorer with both folders and files
- persistent autosave in browser storage
- import/export project JSON files
- large runtime screen (with fullscreen mode) for visual apps/games
- bottom dock tabs for Problems, Output, Terminal, and Debug Console

This is support mode (especially useful on Chromebook). The primary workflow remains the downloadable local compiler.


## Documentation map (new)

- `docs/syntax.md`
- `docs/grammar.md`
- `docs/examples.md`
- `docs/stdlib.md`
- `docs/error_guide.md`
- `docs/roadmap.md`
- `docs/repl.md`
- `docs/game_api.md`

## Package manager scaffold (new)

MagPhos now includes an early package/runtime CLI helper at `tools/scripts/mp.sh`:

- `mp.sh install physics`
- `mp.sh run game.mp`
- `mp.sh repl`

Current `install` behavior is a local placeholder (`.mp_packages/<name>`), designed as the first step toward a full ecosystem package manager.

## Real runtime model (new)

MagPhos now has language-owned runtime behavior rather than relying on JS semantics:

- custom `Value` representation with explicit `TypeKind`
- hierarchical `Environment` scopes (`define`, `assign`, `get`)
- function registration and call arity checks in `RuntimeEngine`
- lexical scope handling for blocks/functions
- dedicated runtime error types (`RuntimeErrorCode` + `RuntimeError`)

Core runtime execution is implemented in `src/runtime/engine.{h,cpp}` and error types in `src/runtime/errors.{h,cpp}`.

## Error messages (new)

Parser diagnostics now include beginner-friendly context:

- exact line + column
- source line snippet
- caret indicator (`^`)
- clear message
- actionable hint

Example style:

```text
Error at line 12, column 9:
  var x =
          ^
Expected expression.
Hint: Did you forget a value, identifier, or parentheses?
```

## Module / import system (new)

MagPhos now parses module dependencies in source code:

- `import math`
- `import game.engine`
- `use "utils.mp"`

Runtime helpers in `src/runtime/module_system.{h,cpp}` resolve and load these forms:

- dotted imports → `baseDir/game/engine.mp`
- use paths → `baseDir/utils.mp`

## Standard library (new)

MagPhos now exposes a native standard library surface:

- **Core**: `len`, `type`, `toString`, `random`, `time`
- **Math**: `sin`, `cos`, `sqrt`, `abs`
- **Strings**: `split`, `replace`, `substring`
- **Arrays**: `push`, `pop`, `map`, `filter`
- **File I/O (native)**: `readFile`, `writeFile`

Implementation is in `src/runtime/stdlib.h` and `src/runtime/stdlib.cpp`.

## Runtime type system (new)

MagPhos runtime now exposes explicit value categories:

- `number`
- `string`
- `boolean`
- `null`
- `function`
- `object/dict`
- `array`
- `map`
- `class`
- `struct`
- `enum`

These are represented in `src/runtime/value.h` via `TypeKind` and `Value`, and stored in `Environment` as typed values instead of plain strings.

## Parser architecture (new)

MagPhos now includes a real front-end pipeline:

- **Tokenizer (`src/lexer`)**: converts source text into typed tokens with line/column tracking.
- **Parser (`src/parser`)**: builds an AST from tokens (functions, blocks, assignments, print/return, and expression statements).
- **Expression grammar** with precedence:
  - unary (`-x`)
  - multiplicative (`*`, `/`)
  - additive (`+`, `-`)
  - grouping/calls (`(expr)`, `name(arg1, arg2)`)
- **Statement terminators**: supports both semicolons (`;`) and newlines.
- **Error recovery**: parser synchronizes at statement boundaries to continue collecting errors after malformed code.

## Build system capabilities (new)

CMake now supports:

- platform detection and `MAGPHOS_PLATFORM_OVERRIDE`
- static + shared core libraries (`MAGPHOS_BUILD_STATIC`, `MAGPHOS_BUILD_SHARED`)
- CLI tool build (`MAGPHOS_BUILD_CLI`)
- REPL tool build (`MAGPHOS_BUILD_REPL`)
- optional WASM target when using Emscripten (`MAGPHOS_BUILD_WASM`)

Example:

```bash
cmake -S . -B build -DMAGPHOS_BUILD_SHARED=ON -DMAGPHOS_BUILD_STATIC=ON
cmake --build build
```

## Test suite (expanded)

The repository now has dedicated test groups:

- `tests/lexer_tests.cpp`
- `tests/parser_tests.cpp`
- `tests/runtime_tests.cpp`
- `tests/error_tests.cpp`
- `tests/example_tests.cpp`

Run all tests with:

```bash
./tools/scripts/run_tests.sh
```

## Syntax overview (current parser)

```txt
import math
use "utils.mp"

var x = 10
const name = "MagPhos"
x = x + 1

fn greet(person) {
  print "Hello " + person
  return person
}

{
  print greet(name)
}
```

Supported top-level declarations and statements (today):

- modules: `import`, `use`
- declarations: `fn`, `var`, `const`
- control flow: `if` / `else`, `while`, `for (...)`
- statements: assignment (`name = expr`), `print`, `return`, block statements (`{ ... }`)
- expression statements (e.g. `greet(name)`)
- expression forms: numbers, strings, booleans, `null`, identifiers, unary (`-`, `not`, `!`), arithmetic, comparisons, equality, logical (`and` / `or`), grouping, calls
- terminators: newline or `;`

Not implemented yet in parser/lexer: `set`, `ask`, `when`, `loop`, `repeat while`, `stop`, `next`, or comment syntax.
