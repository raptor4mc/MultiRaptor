#pragma once

#include <string>

namespace magphos::interpreter {

std::string evaluatePrintStatement(const std::string& line);

// Parses a MagPhos program with lexer+parser and returns a short status message.
std::string analyzeProgram(const std::string& source);

} // namespace magphos::interpreter
