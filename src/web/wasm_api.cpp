#include "magphos_compiler.h"
#include "runtime/interpreter/interpreter.h"
#include "web/htmlxd_preview.h"
#include "web/source_preview.h"

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

std::string render_preview_shell_for_web(const std::string& runOutput, bool forceGameView) {
    return magphos::web::renderPreviewShellHtml(runOutput, forceGameView);
}

std::string render_preview_from_source_for_web(const std::string& source) {
    return magphos::web::renderPreviewFromSource(source);
}

} // namespace

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_BINDINGS(magphos_wasm_api) {
    emscripten::function("compileMagPhos", &compile_for_web);
    emscripten::function("analyzeMagPhos", &analyze_for_web);
    emscripten::function("renderPreviewShell", &render_preview_shell_for_web);
    emscripten::function("renderPreviewFromSource", &render_preview_from_source_for_web);
}
#endif
