#pragma once

#include <string>
#include <vector>

#include "ast/nodes.h"
#include "lexer/lexer.h"

namespace magphos::parser {

struct ParseError {
    std::size_t line;
    std::size_t column;
    std::string message;
};

struct ParseResult {
    ast::Program program;
    std::vector<ParseError> errors;
};

class Parser {
  public:
    ParseResult parse(const std::vector<lexer::Token>& tokens) const;
};

// Legacy helper retained for compatibility.
std::string normalizeLine(const std::string& line);

} // namespace magphos::parser
