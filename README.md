# MultiRaptor

MultiRaptor is a tiny experimental language that tries to feel **easier than Python** while keeping a **JavaScript-like execution model**.

This repo includes:

- A **C++ compiler** (`src/multiraptor_compiler.cpp`) that transpiles `.mp` files to JavaScript.
- A browser **HTML playground** (`web/playground.html`) to write and run MultiRaptor instantly.

## Why MultiRaptor (mp)
Well, i dont have any reason other then its easier then python, and are built on top of c++ so it might be faster then python, and well, its error searching is top tier, and you can make basicly any game inside of it!

## Syntax (simple + readable)

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

## Build the C++ compiler

```bash
g++ -std=c++17 -O2 -o multiraptor src/multiraptor_compiler.cpp
```

## Compile a MultiRaptor program

```bash
./multiraptor main.mp output.js
node output.js
```

## Roadmap
We got this plan on first making this a globaly used language, then keep developing it till i get bored, then il give it to a colluege and i will sleep

## Browser playground

Open:

```txt
web/playground.html
```

in your browser and click **Compile** / **Run**.

## Notes on performance

- The compiler is a fast line-based parser + direct JS emitter.
- Runtime speed follows JavaScript engine performance.
- You can later compile this C++ compiler to **WebAssembly** for sharing one compiler core in browser/native.
