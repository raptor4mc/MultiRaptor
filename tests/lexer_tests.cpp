#include <cassert>

#include "lexer/lexer.h"

int main() {
    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize("import game.engine\nuse \"utils.mp\"\nx = (a + b) * c;\nprint (x + y) / 2\n");

    assert(!tokens.empty());
    assert(tokens[0].type == magphos::lexer::TokenType::Identifier);

    bool sawSemicolon = false;
    bool sawNewline = false;
    bool sawDot = false;
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
    }

    assert(sawSemicolon);
    assert(sawNewline);
    assert(sawDot);
    return 0;
}
