#include "magphos_compiler.h"
#include "runtime/interpreter/interpreter.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include <string>

namespace {

std::string compile_for_web(const std::string& source) {
    return magphos::compileToJavaScript(source);
}

std::string analyze_for_web(const std::string& source) {
    return magphos::interpreter::analyzeProgram(source);
}

} // namespace

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(magphos_wasm_api) {
    emscripten::function("compileMagPhos", &compile_for_web);
    emscripten::function("analyzeMagPhos", &analyze_for_web);
}
#endif
