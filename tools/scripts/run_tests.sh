#!/usr/bin/env bash
set -euo pipefail
c++ -std=c++17 -Iinclude -Isrc tests/lexer_tests.cpp src/lexer/lexer.cpp -o /tmp/lexer_tests
/tmp/lexer_tests
c++ -std=c++17 -Iinclude -Isrc tests/parser_tests.cpp src/parser/parser.cpp src/utils/string_utils.cpp -o /tmp/parser_tests
/tmp/parser_tests
c++ -std=c++17 -Iinclude -Isrc tests/runtime_tests.cpp src/runtime/environment.cpp -o /tmp/runtime_tests
/tmp/runtime_tests
