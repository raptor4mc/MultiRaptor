#pragma once

#include <string>

#include "ast/nodes.h"
#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/environment.h"
#include "runtime/value.h"
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
