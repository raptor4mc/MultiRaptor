#include <cassert>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"

int main() {
    const std::string source = R"(
fn add(a, b) {
  return a + b
}

x = (a + b) * c;
print (x + y) / 2
)";

    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(source);

    magphos::parser::Parser parser;
    const auto result = parser.parse(tokens);

    assert(result.errors.empty());
    assert(!result.program.statements.empty());
    return 0;
}
