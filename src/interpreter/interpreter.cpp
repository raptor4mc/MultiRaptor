#include "interpreter/interpreter.h"

#include <sstream>

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

    std::ostringstream out;
    out << "errors=" << result.errors.size() << " first=" << result.errors.front().line << ':'
        << result.errors.front().column << ' ' << result.errors.front().message;
    return out.str();
}

} // namespace magphos::interpreter
