#pragma once

#include <string>
#include <vector>

namespace magphos::lexer {

enum class TokenType {
    Identifier,
    Number,
    String,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    LeftBracket,
    RightBracket,
    Comma,
    Dot,
    At,
    Semicolon,
    Plus,
    Minus,
    Star,
    Slash,
    Bang,
    BangEqual,
    Equal,
    EqualEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Newline,
    EndOfFile,
    Invalid,
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::size_t line;
    std::size_t column;
};

class Lexer {
  public:
    std::vector<Token> tokenize(const std::string& source) const;
};

// Legacy helper retained for compatibility.
std::vector<std::string> splitWhitespace(const std::string& text);

} // namespace magphos::lexer
