cmake_minimum_required(VERSION 3.16)
project(MagPhos VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(magphos_core STATIC
  src/ast/nodes.cpp
  src/interpreter/interpreter.cpp
  src/lexer/lexer.cpp
  src/parser/parser.cpp
  src/runtime/environment.cpp
  src/runtime/enviroment.cpp
  src/runtime/value.cpp
  src/utils/error.cpp
  src/utils/file_utils.cpp
  src/utils/string_utils.cpp
  src/main.cpp
)

target_include_directories(magphos_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(MagPhos_compiler
  src/MagPhos_compiler.cpp
)

target_link_libraries(MagPhos_compiler PRIVATE magphos_core)

install(TARGETS MagPhos_compiler RUNTIME DESTINATION bin)
