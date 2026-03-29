#pragma once

#include <string>

#include "compiler/ast/nodes.h"
#include "runtime/interpreter/interpreter.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "runtime/engine/environment.h"
#include "runtime/engine/engine.h"
#include "runtime/engine/errors.h"
#include "runtime/engine/value.h"
#include "runtime/stdlib/stdlib.h"
#include "runtime/engine/module_system.h"
#include "magphos_compiler.h"
#include "utils/file_utils"
#include "utils/string_utils.h"

namespace magphos {

inline std::string languageName() {
    return "MagPhos";
}

inline std::string extensionName() {
    return ".mp";
}

inline std::string extensionMeaning() {
    return "MagnesiumPhosphorus";
}

} // namespace magphos
