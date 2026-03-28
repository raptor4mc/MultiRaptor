# MagPhos

MagPhos is the language name.

- File extension / short name: **`.mp`**
- Meaning of `mp`: **MagnesiumPhosphorus**

## Download places (versioned)

> A new row should be added on every update so users can download specific versions.

| Version | Date | Download |
|---|---|---|
| v0.1.3 | 2026-03-28 | https://github.com/<owner>/<repo>/archive/refs/heads/work.zip |
| v0.1.2 | 2026-03-28 | https://github.com/<owner>/<repo>/archive/refs/heads/main.zip |
| v0.1.1 | 2026-03-28 | https://github.com/<owner>/<repo>/releases |

Replace `<owner>/<repo>` with your GitHub repository path.

## Folder structure

```txt
/src
  main.cpp
  lexer/lexer.cpp, lexer.h
  parser/parser.cpp, parser.h
  ast/nodes.cpp, nodes.h
  runtime/value.cpp, value.h, environment.cpp, environment.h
  interpreter/interpreter.cpp, interpreter.h
  utils/string_utils.cpp, file_utils.cpp, error.cpp

/include
  magphos.h
  multiraptor.h

/platform
  linux/, windows/, macos/, chromeos/, android/

/tests
  lexer_tests.cpp, parser_tests.cpp, runtime_tests.cpp

/examples
  hello_world.mp, loops.mp, functions.mp

/docs
  syntax.md, roadmap.md, internals/*

/web
  playground.html, playground.js, playground.css

/tools
  scripts/*, generators/*
```

## Build and run

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/MagPhos_compiler main.mp output.js
node output.js
```

## Optional manual compile

```bash
g++ -std=c++17 -Iinclude -Isrc -O2 -o MagPhos_compiler src/MagPhos_compiler.cpp src/ast/nodes.cpp src/interpreter/interpreter.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/runtime/environment.cpp src/runtime/value.cpp src/utils/error.cpp src/utils/file_utils.cpp src/utils/string_utils.cpp src/main.cpp
```

## Browser playground

Open `web/playground.html` in your browser.
