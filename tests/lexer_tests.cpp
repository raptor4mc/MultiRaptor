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
        "}\n"
        "// comment line\n"
        "# comment line\n"
        "arr = [1, 2, 3]\n");

    assert(!tokens.empty());
    assert(tokens[0].type == magphos::lexer::TokenType::Identifier);

    bool sawSemicolon = false;
    bool sawNewline = false;
    bool sawDot = false;
    bool sawGreaterEqual = false;
    bool sawBangEqual = false;
    bool sawLeftBracket = false;
    bool sawRightBracket = false;
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
        if (token.type == magphos::lexer::TokenType::LeftBracket) {
            sawLeftBracket = true;
        }
        if (token.type == magphos::lexer::TokenType::RightBracket) {
            sawRightBracket = true;
        }
    }

    assert(sawSemicolon);
    assert(sawNewline);
    assert(sawDot);
    assert(sawGreaterEqual);
    assert(sawBangEqual);
    assert(sawLeftBracket);
    assert(sawRightBracket);
    return 0;
}
