#include "interpreter/interpreter.h"

#include "lexer/lexer.h"
#include "parser/parser.h"

namespace magphos::interpreter {

std::string evaluatePrintStatement(const std::string& line) {
    return line;
}

std::string analyzeProgram(const std::string& source) {
    lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(source);

    parser::Parser parser;
    const auto result = parser.parse(tokens);

    if (result.errors.empty()) {
        return "ok";
    }

    return parser::renderErrors(result.errors, source);
}

} // namespace magphos::interpreter
