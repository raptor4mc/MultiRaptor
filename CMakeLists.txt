cmake_minimum_required(VERSION 3.16)
project(MagPhos VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(MAGPHOS_BUILD_SHARED "Build shared MagPhos core library" ON)
option(MAGPHOS_BUILD_STATIC "Build static MagPhos core library" ON)
option(MAGPHOS_BUILD_CLI "Build magphos_cli tool" ON)
option(MAGPHOS_BUILD_REPL "Build interactive REPL tool" ON)
option(MAGPHOS_BUILD_WASM "Build WebAssembly target when using Emscripten" OFF)
set(MAGPHOS_PLATFORM_OVERRIDE "" CACHE STRING "Override detected platform (linux/windows/macos/android/chromeos/wasm)")

set(MAGPHOS_PLATFORM "")
if(MAGPHOS_PLATFORM_OVERRIDE)
  set(MAGPHOS_PLATFORM "${MAGPHOS_PLATFORM_OVERRIDE}")
elseif(EMSCRIPTEN)
  set(MAGPHOS_PLATFORM "wasm")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(MAGPHOS_PLATFORM "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(MAGPHOS_PLATFORM "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(MAGPHOS_PLATFORM "macos")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(MAGPHOS_PLATFORM "android")
else()
  set(MAGPHOS_PLATFORM "unknown")
endif()

message(STATUS "MagPhos platform: ${MAGPHOS_PLATFORM}")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/magphos_build_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/generated/magphos_build_config.h
  @ONLY
)

set(MAGPHOS_CORE_SOURCES
  src/ast/nodes.cpp
  src/interpreter/interpreter.cpp
  src/lexer/lexer.cpp
  src/parser/parser.cpp
  src/runtime/environment.cpp
  src/runtime/errors.cpp
  src/runtime/value.cpp
  src/runtime/stdlib.cpp
  src/runtime/module_system.cpp
  src/runtime/engine.cpp
  src/utils/error.cpp
  src/utils/file_utils.cpp
  src/utils/string_utils.cpp
  src/main.cpp
  src/compiler_compiler.cpp
)

add_library(magphos_core_obj OBJECT ${MAGPHOS_CORE_SOURCES})
target_include_directories(magphos_core_obj
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)
set_target_properties(magphos_core_obj PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MAGPHOS_BUILD_STATIC)
  add_library(magphos_core_static STATIC $<TARGET_OBJECTS:magphos_core_obj>)
  target_include_directories(magphos_core_static PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
  )
  set_target_properties(magphos_core_static PROPERTIES OUTPUT_NAME magphos_core)
endif()

if(MAGPHOS_BUILD_SHARED)
  add_library(magphos_core_shared SHARED $<TARGET_OBJECTS:magphos_core_obj>)
  target_include_directories(magphos_core_shared PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
  )
  set_target_properties(magphos_core_shared PROPERTIES OUTPUT_NAME magphos_core)
endif()

add_executable(MagPhos_compiler src/MagPhos_compiler.cpp)
if(TARGET magphos_core_static)
  target_link_libraries(MagPhos_compiler PRIVATE magphos_core_static)
else()
  target_link_libraries(MagPhos_compiler PRIVATE magphos_core_shared)
endif()

if(MAGPHOS_BUILD_CLI)
  add_executable(magphos_cli src/tools/magphos_cli.cpp)
  if(TARGET magphos_core_static)
    target_link_libraries(magphos_cli PRIVATE magphos_core_static)
  else()
    target_link_libraries(magphos_cli PRIVATE magphos_core_shared)
  endif()
endif()

if(MAGPHOS_BUILD_REPL)
  add_executable(magphos_repl src/tools/magphos_repl.cpp)
  if(TARGET magphos_core_static)
    target_link_libraries(magphos_repl PRIVATE magphos_core_static)
  else()
    target_link_libraries(magphos_repl PRIVATE magphos_core_shared)
  endif()
endif()

if(MAGPHOS_BUILD_WASM)
  if(NOT EMSCRIPTEN)
    message(WARNING "MAGPHOS_BUILD_WASM=ON but Emscripten was not detected. Skipping wasm target.")
  else()
    add_executable(MagPhos_wasm src/MagPhos_compiler.cpp src/web/wasm_api.cpp)
    if(TARGET magphos_core_static)
      target_link_libraries(MagPhos_wasm PRIVATE magphos_core_static)
    else()
      target_link_libraries(MagPhos_wasm PRIVATE magphos_core_shared)
    endif()
    set_target_properties(MagPhos_wasm PROPERTIES
      OUTPUT_NAME "magphos_wasm"
      SUFFIX ".js"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
    )
    target_link_options(MagPhos_wasm PRIVATE
      "-sEXPORT_ES6=1"
      "-sMODULARIZE=1"
      "-sENVIRONMENT=web"
      "-sALLOW_MEMORY_GROWTH=1"
    )
  endif()
endif()

install(TARGETS MagPhos_compiler RUNTIME DESTINATION bin)
if(TARGET magphos_cli)
  install(TARGETS magphos_cli RUNTIME DESTINATION bin)
endif()
if(TARGET magphos_repl)
  install(TARGETS magphos_repl RUNTIME DESTINATION bin)
endif()
if(TARGET magphos_core_static)
  install(TARGETS magphos_core_static ARCHIVE DESTINATION lib)
endif()
if(TARGET magphos_core_shared)
  install(TARGETS magphos_core_shared LIBRARY DESTINATION lib)
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/magphos_build_config.h DESTINATION include)
