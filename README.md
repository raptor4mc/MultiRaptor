# MultiRaptor

MultiRaptor is a tiny experimental language that tries to feel **easier than Python** while keeping a **JavaScript-like execution model**.

This repo includes:

- A **C++ compiler** (`src/multiraptor_compiler.cpp`) that transpiles `.mp` files to JavaScript.
- A browser **HTML playground** (`web/playground.html`) to write and run MultiRaptor instantly.

## Syntax

```txt
var x = 10
const name = "Raptor"

fn greet(person) {
  print "Hello " + person
}

loop 3 {
  greet(name)
}
```

Supported statements:

- `var name = expr`
- `const name = expr`
- `print expr`
- `if condition { ... }`
- `else { ... }`
- `loop count { ... }`
- `fn name(args) { ... }`
- `return expr`

## Build the C++ compiler

```bash
g++ -std=c++17 -O2 -o multiraptor src/multiraptor_compiler.cpp
```

## Compile a MultiRaptor program

```bash
./multiraptor main.mp output.js
node output.js
```

## Browser playground

Open:

```txt
web/playground.html
```

in your browser and click **Compile** / **Run**.

## Notes on performance

- The language is designed to compile quickly (line-based parser + direct JS emission).
- Runtime speed follows JavaScript engine performance.
- You can later compile this C++ compiler to **WebAssembly** if you want the browser version to run the same compiler binary.
