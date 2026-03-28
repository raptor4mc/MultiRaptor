#include <cassert>

#include "lexer/lexer.h"

int main() {
    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize("x = (a + b) * c;\nprint (x + y) / 2\n");

    assert(!tokens.empty());
    assert(tokens[0].type == magphos::lexer::TokenType::Identifier);

    bool sawSemicolon = false;
    bool sawNewline = false;
    for (const auto& token : tokens) {
        if (token.type == magphos::lexer::TokenType::Semicolon) {
            sawSemicolon = true;
        }
        if (token.type == magphos::lexer::TokenType::Newline) {
            sawNewline = true;
        }
    }

    assert(sawSemicolon);
    assert(sawNewline);
    return 0;
}
