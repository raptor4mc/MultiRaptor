#!/usr/bin/env bash
set -euo pipefail

COMMON_SRCS=(
  src/interpreter/interpreter.cpp
  src/lexer/lexer.cpp
  src/parser/parser.cpp
  src/ast/nodes.cpp
  src/runtime/environment.cpp
  src/runtime/errors.cpp
  src/runtime/value.cpp
  src/runtime/stdlib.cpp
  src/runtime/module_system.cpp
  src/runtime/engine.cpp
  src/semantic/analyzer.cpp
  src/utils/string_utils.cpp
)

mkdir -p /tmp/magphos_tests

g++ -std=c++17 -Iinclude -Isrc tests/lexer_tests.cpp src/lexer/lexer.cpp -o /tmp/magphos_tests/lexer_tests
/tmp/magphos_tests/lexer_tests

g++ -std=c++17 -Iinclude -Isrc tests/parser_tests.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/ast/nodes.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/parser_tests
/tmp/magphos_tests/parser_tests

g++ -std=c++17 -Iinclude -Isrc tests/runtime_tests.cpp "${COMMON_SRCS[@]}" -o /tmp/magphos_tests/runtime_tests
/tmp/magphos_tests/runtime_tests

g++ -std=c++17 -Iinclude -Isrc tests/error_tests.cpp src/interpreter/interpreter.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/ast/nodes.cpp src/semantic/analyzer.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/error_tests
/tmp/magphos_tests/error_tests

g++ -std=c++17 -Iinclude -Isrc tests/example_tests.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/ast/nodes.cpp src/utils/string_utils.cpp -o /tmp/magphos_tests/example_tests
/tmp/magphos_tests/example_tests

echo "All MagPhos tests passed."

# REPL smoke test
g++ -std=c++17 -Iinclude -Isrc src/tools/magphos_repl.cpp "${COMMON_SRCS[@]}" -o /tmp/magphos_tests/magphos_repl
printf "var x = 10\n:quit\n" | /tmp/magphos_tests/magphos_repl | grep -q "ok"
