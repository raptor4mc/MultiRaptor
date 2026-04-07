#include "compiler/lexer/lexer.h"

#include <cctype>
#include <sstream>

namespace magphos::lexer {

namespace {

void pushSimple(std::vector<Token>& tokens,
                TokenType type,
                char ch,
                std::size_t line,
                std::size_t column) {
    tokens.push_back(Token{type, std::string(1, ch), line, column});
}

} // namespace

std::vector<Token> Lexer::tokenize(const std::string& source) const {
    std::vector<Token> tokens;
    std::size_t index = 0;
    std::size_t line = 1;
    std::size_t column = 1;

    const auto atEnd = [&]() { return index >= source.size(); };

    while (!atEnd()) {
        const char c = source[index];
        const std::size_t startColumn = column;

        if (c == ' ' || c == '\t' || c == '\r') {
            ++index;
            ++column;
            continue;
        }

        if (c == '\n') {
            tokens.push_back(Token{TokenType::Newline, "\\n", line, startColumn});
            ++index;
            ++line;
            column = 1;
            continue;
        }

        if (c == '#') {
            while (!atEnd() && source[index] != '\n') {
                ++index;
                ++column;
            }
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            const std::size_t start = index;
            while (!atEnd()) {
                const char look = source[index];
                if (!std::isalnum(static_cast<unsigned char>(look)) && look != '_') {
                    break;
                }
                ++index;
                ++column;
            }
            tokens.push_back(Token{TokenType::Identifier, source.substr(start, index - start), line, startColumn});
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            const std::size_t start = index;
            while (!atEnd() && std::isdigit(static_cast<unsigned char>(source[index]))) {
                ++index;
                ++column;
            }
            if (!atEnd() && source[index] == '.') {
                ++index;
                ++column;
                while (!atEnd() && std::isdigit(static_cast<unsigned char>(source[index]))) {
                    ++index;
                    ++column;
                }
            }
            tokens.push_back(Token{TokenType::Number, source.substr(start, index - start), line, startColumn});
            continue;
        }

        if (c == '"') {
            ++index;
            ++column;
            std::string value;
            bool terminated = false;
            while (!atEnd()) {
                const char current = source[index];
                if (current == '"') {
                    terminated = true;
                    ++index;
                    ++column;
                    break;
                }
                if (current == '\n') {
                    break;
                }
                value.push_back(current);
                ++index;
                ++column;
            }

            if (!terminated) {
                tokens.push_back(Token{TokenType::Invalid, "Unterminated string", line, startColumn});
            } else {
                tokens.push_back(Token{TokenType::String, value, line, startColumn});
            }
            continue;
        }

        switch (c) {
            case '(':
                pushSimple(tokens, TokenType::LeftParen, c, line, startColumn);
                break;
            case ')':
                pushSimple(tokens, TokenType::RightParen, c, line, startColumn);
                break;
            case '{':
                pushSimple(tokens, TokenType::LeftBrace, c, line, startColumn);
                break;
            case '}':
                pushSimple(tokens, TokenType::RightBrace, c, line, startColumn);
                break;
            case '[':
                pushSimple(tokens, TokenType::LeftBracket, c, line, startColumn);
                break;
            case ']':
                pushSimple(tokens, TokenType::RightBracket, c, line, startColumn);
                break;
            case ',':
                pushSimple(tokens, TokenType::Comma, c, line, startColumn);
                break;
            case '.':
                pushSimple(tokens, TokenType::Dot, c, line, startColumn);
                break;
            case '@':
                pushSimple(tokens, TokenType::At, c, line, startColumn);
                break;
            case ';':
                pushSimple(tokens, TokenType::Semicolon, c, line, startColumn);
                break;
            case '+':
                pushSimple(tokens, TokenType::Plus, c, line, startColumn);
                break;
            case '-':
                pushSimple(tokens, TokenType::Minus, c, line, startColumn);
                break;
            case '*':
                pushSimple(tokens, TokenType::Star, c, line, startColumn);
                break;
            case '/':
                if (index + 1 < source.size() && source[index + 1] == '/') {
                    ++index;
                    ++column;
                    while (index + 1 <= source.size() && !atEnd() && source[index] != '\n') {
                        ++index;
                        ++column;
                    }
                    continue;
                }
                pushSimple(tokens, TokenType::Slash, c, line, startColumn);
                break;
            case '!':
                if (index + 1 < source.size() && source[index + 1] == '=') {
                    tokens.push_back(Token{TokenType::BangEqual, "!=", line, startColumn});
                    ++index;
                    ++column;
                } else {
                    pushSimple(tokens, TokenType::Bang, c, line, startColumn);
                }
                break;
            case '=':
                if (index + 1 < source.size() && source[index + 1] == '=') {
                    tokens.push_back(Token{TokenType::EqualEqual, "==", line, startColumn});
                    ++index;
                    ++column;
                } else {
                    pushSimple(tokens, TokenType::Equal, c, line, startColumn);
                }
                break;
            case '<':
                if (index + 1 < source.size() && source[index + 1] == '=') {
                    tokens.push_back(Token{TokenType::LessEqual, "<=", line, startColumn});
                    ++index;
                    ++column;
                } else {
                    pushSimple(tokens, TokenType::Less, c, line, startColumn);
                }
                break;
            case '>':
                if (index + 1 < source.size() && source[index + 1] == '=') {
                    tokens.push_back(Token{TokenType::GreaterEqual, ">=", line, startColumn});
                    ++index;
                    ++column;
                } else {
                    pushSimple(tokens, TokenType::Greater, c, line, startColumn);
                }
                break;
            default:
                tokens.push_back(Token{TokenType::Invalid, std::string("Unexpected character: ") + c, line, startColumn});
                break;
        }

        ++index;
        ++column;
    }

    tokens.push_back(Token{TokenType::EndOfFile, "", line, column});
    return tokens;
}

std::vector<std::string> splitWhitespace(const std::string& text) {
    std::vector<std::string> tokens;
    std::stringstream stream(text);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace magphos::lexer
