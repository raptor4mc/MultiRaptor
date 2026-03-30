#!/usr/bin/env bash
set -euo pipefail

COMMON_SRCS=(
  src/runtime/interpreter/interpreter.cpp
  src/compiler/lexer/lexer.cpp
  src/compiler/parser/parser.cpp
  src/compiler/ast/nodes.cpp
  src/runtime/engine/environment.cpp
  src/runtime/engine/errors.cpp
  src/runtime/engine/value.cpp
  src/runtime/stdlib/stdlib.cpp
  src/runtime/engine/module_system.cpp
  src/runtime/engine/engine.cpp
  src/compiler/semantic/analyzer.cpp
  src/utils/string_utils.cpp
)

mkdir -p /tmp/magphos_tests

g++ -std=c++17 -Iinclude -Isrc tests/lexer_tests.cpp src/compiler/lexer/lexer.cpp -o /tmp/magphos_tests/lexer_tests
/tmp/magphos_tests/lexer_tests

g++ -std=c++17 -Iinclude -Isrc tests/parser_tests.cpp src/compiler/lexer/lexer.cpp src/compiler/parser/parser.cpp src/compiler/ast/nodes.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/parser_tests
/tmp/magphos_tests/parser_tests

g++ -std=c++17 -Iinclude -Isrc tests/runtime_tests.cpp "${COMMON_SRCS[@]}" -o /tmp/magphos_tests/runtime_tests
/tmp/magphos_tests/runtime_tests

g++ -std=c++17 -Iinclude -Isrc src/tools/magphos_cli.cpp "${COMMON_SRCS[@]}" -o /tmp/magphos_tests/magphos_cli
g++ -std=c++17 -Iinclude -Isrc tests/cli_tests.cpp -o /tmp/magphos_tests/cli_tests
/tmp/magphos_tests/cli_tests

g++ -std=c++17 -Iinclude -Isrc tests/error_tests.cpp src/runtime/interpreter/interpreter.cpp src/compiler/lexer/lexer.cpp src/compiler/parser/parser.cpp src/compiler/ast/nodes.cpp src/compiler/semantic/analyzer.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/error_tests
/tmp/magphos_tests/error_tests

g++ -std=c++17 -Iinclude -Isrc tests/example_tests.cpp src/compiler/lexer/lexer.cpp src/compiler/parser/parser.cpp src/compiler/ast/nodes.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/example_tests
/tmp/magphos_tests/example_tests

echo "All MagPhos tests passed."

# REPL smoke test
g++ -std=c++17 -Iinclude -Isrc src/tools/magphos_repl.cpp "${COMMON_SRCS[@]}" -o /tmp/magphos_tests/magphos_repl
printf "var x = 10\n:quit\n" | /tmp/magphos_tests/magphos_repl | grep -q "ok"
