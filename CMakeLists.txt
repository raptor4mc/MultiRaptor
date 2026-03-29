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
set(MAGPHOS_PLATFORM_OVERRIDE "" CACHE STRING "Override detected platform (linux/windows/macos/android/chromeos/vscode/wasm)")

set(MAGPHOS_PLATFORM "")
if(MAGPHOS_PLATFORM_OVERRIDE)
  set(MAGPHOS_PLATFORM "${MAGPHOS_PLATFORM_OVERRIDE}")
elseif(EMSCRIPTEN)
  set(MAGPHOS_PLATFORM "wasm")
elseif(DEFINED ENV{VSCODE_PID} AND NOT "$ENV{VSCODE_PID}" STREQUAL "")
  set(MAGPHOS_PLATFORM "vscode")
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
  src/compiler/ast/nodes.cpp
  src/runtime/interpreter/interpreter.cpp
  src/compiler/lexer/lexer.cpp
  src/compiler/parser/parser.cpp
  src/runtime/engine/environment.cpp
  src/runtime/engine/errors.cpp
  src/runtime/engine/value.cpp
  src/runtime/stdlib/stdlib.cpp
  src/runtime/engine/module_system.cpp
  src/runtime/engine/engine.cpp
  src/compiler/semantic/analyzer.cpp
  src/htmlxd/dom.cpp
  src/web/htmlxd_preview.cpp
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

option(MAGPHOS_BUILD_VSCODE "Build unified VSCode launcher" ON)

if(MAGPHOS_BUILD_VSCODE)
  add_executable(magphos_vscode src/platform/vscode/main.cpp)
  target_include_directories(magphos_vscode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/generated
  )
  if(TARGET magphos_core_static)
    target_link_libraries(magphos_vscode PRIVATE magphos_core_static)
  else()
    target_link_libraries(magphos_vscode PRIVATE magphos_core_shared)
  endif()
endif()

if(MAGPHOS_BUILD_WASM)
  if(NOT EMSCRIPTEN)
    message(FATAL_ERROR
      "MAGPHOS_BUILD_WASM=ON requires Emscripten, but this configure was not launched via emcmake.\n"
      "Use: emcmake cmake -S . -B build-web -DMAGPHOS_BUILD_WASM=ON -DMAGPHOS_BUILD_CLI=OFF -DMAGPHOS_BUILD_REPL=OFF"
    )
  else()
    add_executable(MagPhos_wasm src/web/wasm_api.cpp)
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
      "--bind"
      "-sEXPORT_ES6=1"
      "-sMODULARIZE=1"
      "-sEXPORT_NAME=MagPhosWasmFactory"
      "-sENVIRONMENT=web"
      "-sALLOW_MEMORY_GROWTH=1"
      "-sFILESYSTEM=0"
    )
    add_custom_command(TARGET MagPhos_wasm POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:MagPhos_wasm>/magphos_wasm.wasm"
        "${CMAKE_SOURCE_DIR}/web/magphos_wasm.wasm.64"
      VERBATIM
    )

    add_executable(MagPhos_wasm_singlefile src/web/wasm_api.cpp)
    if(TARGET magphos_core_static)
      target_link_libraries(MagPhos_wasm_singlefile PRIVATE magphos_core_static)
    else()
      target_link_libraries(MagPhos_wasm_singlefile PRIVATE magphos_core_shared)
    endif()
    set_target_properties(MagPhos_wasm_singlefile PROPERTIES
      OUTPUT_NAME "magphos_wasm_singlefile"
      SUFFIX ".js"
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web"
    )
    target_link_options(MagPhos_wasm_singlefile PRIVATE
      "--bind"
      "-sENVIRONMENT=web"
      "-sALLOW_MEMORY_GROWTH=1"
      "-sFILESYSTEM=0"
      "-sSINGLE_FILE=1"
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
if(TARGET magphos_vscode)
  install(TARGETS magphos_vscode RUNTIME DESTINATION bin)
endif()
if(TARGET magphos_core_static)
  install(TARGETS magphos_core_static ARCHIVE DESTINATION lib)
endif()
if(TARGET magphos_core_shared)
  install(TARGETS magphos_core_shared LIBRARY DESTINATION lib)
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/generated/magphos_build_config.h DESTINATION include)
