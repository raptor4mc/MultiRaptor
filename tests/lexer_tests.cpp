#include <cassert>

#include "lexer/lexer.h"

int main() {
    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(
        "import game.engine\n"
        "use \"utils.mp\"\n"
        "x = (a + b) * c;\n"
        "print (x + y) / 2\n"
        "if x >= 2 and x != 5 {\n"
        "  print not false\n"
        "}\n");

    assert(!tokens.empty());
    assert(tokens[0].type == magphos::lexer::TokenType::Identifier);

    bool sawSemicolon = false;
    bool sawNewline = false;
    bool sawDot = false;
    bool sawGreaterEqual = false;
    bool sawBangEqual = false;
    for (const auto& token : tokens) {
        if (token.type == magphos::lexer::TokenType::Semicolon) {
            sawSemicolon = true;
        }
        if (token.type == magphos::lexer::TokenType::Newline) {
            sawNewline = true;
        }
        if (token.type == magphos::lexer::TokenType::Dot) {
            sawDot = true;
        }
        if (token.type == magphos::lexer::TokenType::GreaterEqual) {
            sawGreaterEqual = true;
        }
        if (token.type == magphos::lexer::TokenType::BangEqual) {
            sawBangEqual = true;
        }
    }

    assert(sawSemicolon);
    assert(sawNewline);
    assert(sawDot);
    assert(sawGreaterEqual);
    assert(sawBangEqual);
    return 0;
}
