#include "interpreter/interpreter.h"

#include "parser/parser.h"

namespace magphos::interpreter {

std::string evaluatePrintStatement(const std::string& line) {
    return parser::normalizeLine(line);
}

} // namespace magphos::interpreter
