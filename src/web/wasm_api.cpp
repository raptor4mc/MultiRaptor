#include "magphos_compiler.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#endif

#include <string>

namespace {

std::string compile_for_web(const std::string& source) {
    return magphos::compileToJavaScript(source);
}

} // namespace

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(magphos_wasm_api) {
    emscripten::function("compileMagPhos", &compile_for_web);
}
#endif
