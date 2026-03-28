#include "parser/parser.h"

#include <memory>
#include <sstream>
#include <optional>
#include <utility>

#include "utils/string_utils.h"

namespace magphos::parser {

namespace {
using lexer::Token;
using lexer::TokenType;

class ParserImpl {
  public:
    explicit ParserImpl(const std::vector<Token>& tokens) : tokens_(tokens) {}

    ParseResult parseProgram() {
        ParseResult result;
        while (!isAtEnd()) {
            skipTerminators();
            if (isAtEnd()) {
                break;
            }

            auto statement = parseDeclaration();
            if (statement.has_value()) {
                result.program.statements.push_back(std::move(statement.value()));
            } else {
                result.errors.push_back(lastError_);
                synchronize();
            }
        }
        return result;
    }

  private:
    const std::vector<Token>& tokens_;
    std::size_t current_ = 0;
    ParseError lastError_{1, 1, "Unknown parse error", "Check the syntax near this token."};


    static std::string makeHint(const std::string& message) {
        if (message.find("Expected expression") != std::string::npos) {
            return "Did you forget a value, identifier, or parentheses?";
        }
        if (message.find("Expected ';' or newline") != std::string::npos) {
            return "End the statement with a newline or ';'.";
        }
        if (message.find("Expected ')'") != std::string::npos) {
            return "Check for an opening '(' without a matching closing ')'.";
        }
        if (message.find("Expected '}'") != std::string::npos) {
            return "Check for a block opened with '{' that was never closed.";
        }
        if (message.find("Expected quoted path after 'use'") != std::string::npos) {
            return "Use double quotes, for example: use \"utils.mp\"";
        }
        if (message.find("Expected module name after 'import'") != std::string::npos) {
            return "Use identifiers like import math or import game.engine.";
        }
        return "Review the syntax near the marked position.";
    }

    bool isAtEnd() const { return peek().type == TokenType::EndOfFile; }

    const Token& peek() const { return tokens_[current_]; }

    const Token& previous() const { return tokens_[current_ - 1]; }

    const Token& advance() {
        if (!isAtEnd()) {
            ++current_;
        }
        return previous();
    }

    bool check(TokenType type) const { return !isAtEnd() && peek().type == type; }

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool matchKeyword(const std::string& keyword) {
        if (!check(TokenType::Identifier)) {
            return false;
        }
        if (peek().lexeme != keyword) {
            return false;
        }
        advance();
        return true;
    }

    bool checkKeyword(const std::string& keyword) const {
        return check(TokenType::Identifier) && peek().lexeme == keyword;
    }

    void skipTerminators() {
        while (match(TokenType::Semicolon) || match(TokenType::Newline)) {
        }
    }

    std::optional<ast::Statement> parseDeclaration() {
        if (matchKeyword("import")) {
            return parseImportStatement();
        }
        if (matchKeyword("use")) {
            return parseUseStatement();
        }
        if (matchKeyword("fn")) {
            return parseFunctionDeclaration();
        }
        return parseStatement();
    }


    std::optional<ast::Statement> parseImportStatement() {
        if (!check(TokenType::Identifier)) {
            return errorAtCurrent("Expected module name after 'import'.");
        }

        ast::Statement statement;
        statement.kind = ast::StmtKind::Import;
        statement.name = advance().lexeme;

        while (match(TokenType::Dot)) {
            if (!check(TokenType::Identifier)) {
                return errorAtCurrent("Expected identifier after '.'.");
            }
            statement.name += "." + advance().lexeme;
        }

        consumeTerminator("Expected ';' or newline after import.");
        return statement;
    }

    std::optional<ast::Statement> parseUseStatement() {
        if (!check(TokenType::String)) {
            return errorAtCurrent("Expected quoted path after 'use'.");
        }

        ast::Statement statement;
        statement.kind = ast::StmtKind::Use;
        statement.name = advance().lexeme;
        consumeTerminator("Expected ';' or newline after use.");
        return statement;
    }

    std::optional<ast::Statement> parseFunctionDeclaration() {
        if (!check(TokenType::Identifier)) {
            return errorAtCurrent("Expected function name after 'fn'.");
        }

        ast::Statement fn;
        fn.kind = ast::StmtKind::Function;
        fn.name = advance().lexeme;

        if (!match(TokenType::LeftParen)) {
            return errorAtCurrent("Expected '(' after function name.");
        }

        if (!check(TokenType::RightParen)) {
            do {
                if (!check(TokenType::Identifier)) {
                    return errorAtCurrent("Expected parameter name.");
                }
                fn.params.push_back(advance().lexeme);
            } while (match(TokenType::Comma));
        }

        if (!match(TokenType::RightParen)) {
            return errorAtCurrent("Expected ')' after parameters.");
        }

        auto block = parseBlockStatement();
        if (!block.has_value()) {
            return std::nullopt;
        }

        fn.body = std::move(block->body);
        return fn;
    }

    std::optional<ast::Statement> parseStatement() {
        if (matchKeyword("print")) {
            return parseSingleExprStatement(ast::StmtKind::Print);
        }
        if (matchKeyword("return")) {
            return parseSingleExprStatement(ast::StmtKind::Return);
        }
        if (check(TokenType::LeftBrace)) {
            return parseBlockStatement();
        }
        return parseAssignmentOrExpression();
    }

    std::optional<ast::Statement> parseBlockStatement() {
        if (!match(TokenType::LeftBrace)) {
            return errorAtCurrent("Expected '{' to start block.");
        }

        ast::Statement block;
        block.kind = ast::StmtKind::Block;

        while (!check(TokenType::RightBrace) && !isAtEnd()) {
            skipTerminators();
            if (check(TokenType::RightBrace)) {
                break;
            }

            auto statement = parseDeclaration();
            if (!statement.has_value()) {
                return std::nullopt;
            }
            block.body.push_back(std::move(statement.value()));
        }

        if (!match(TokenType::RightBrace)) {
            return errorAtCurrent("Expected '}' after block.");
        }

        skipTerminators();
        return block;
    }

    std::optional<ast::Statement> parseSingleExprStatement(ast::StmtKind kind) {
        auto expr = parseExpression();
        if (!expr) {
            return std::nullopt;
        }

        ast::Statement statement;
        statement.kind = kind;
        statement.expression = std::move(expr);
        consumeTerminator("Expected ';' or newline after statement.");
        return statement;
    }

    std::optional<ast::Statement> parseAssignmentOrExpression() {
        if (check(TokenType::Identifier) && peekNext().type == TokenType::Equal) {
            ast::Statement assignment;
            assignment.kind = ast::StmtKind::Assignment;
            assignment.name = advance().lexeme;
            advance(); // '='

            auto expr = parseExpression();
            if (!expr) {
                return std::nullopt;
            }
            assignment.expression = std::move(expr);
            consumeTerminator("Expected ';' or newline after assignment.");
            return assignment;
        }

        auto expr = parseExpression();
        if (!expr) {
            return std::nullopt;
        }

        ast::Statement statement;
        statement.kind = ast::StmtKind::Expression;
        statement.expression = std::move(expr);
        consumeTerminator("Expected ';' or newline after expression.");
        return statement;
    }

    const Token& peekNext() const {
        if (current_ + 1 >= tokens_.size()) {
            return tokens_.back();
        }
        return tokens_[current_ + 1];
    }

    std::unique_ptr<ast::Expr> parseExpression() { return parseAddition(); }

    std::unique_ptr<ast::Expr> parseAddition() {
        auto left = parseMultiplication();
        if (!left) {
            return nullptr;
        }

        while (match(TokenType::Plus) || match(TokenType::Minus)) {
            const Token op = previous();
            auto right = parseMultiplication();
            if (!right) {
                return nullptr;
            }

            auto binary = std::make_unique<ast::Expr>();
            binary->kind = ast::ExprKind::Binary;
            binary->value = op.lexeme;
            binary->children.push_back(std::move(left));
            binary->children.push_back(std::move(right));
            left = std::move(binary);
        }

        return left;
    }

    std::unique_ptr<ast::Expr> parseMultiplication() {
        auto left = parseUnary();
        if (!left) {
            return nullptr;
        }

        while (match(TokenType::Star) || match(TokenType::Slash)) {
            const Token op = previous();
            auto right = parseUnary();
            if (!right) {
                return nullptr;
            }

            auto binary = std::make_unique<ast::Expr>();
            binary->kind = ast::ExprKind::Binary;
            binary->value = op.lexeme;
            binary->children.push_back(std::move(left));
            binary->children.push_back(std::move(right));
            left = std::move(binary);
        }

        return left;
    }

    std::unique_ptr<ast::Expr> parseUnary() {
        if (match(TokenType::Minus)) {
            auto operand = parseUnary();
            if (!operand) {
                return nullptr;
            }

            auto unary = std::make_unique<ast::Expr>();
            unary->kind = ast::ExprKind::Unary;
            unary->value = "-";
            unary->children.push_back(std::move(operand));
            return unary;
        }

        return parseCall();
    }

    std::unique_ptr<ast::Expr> parseCall() {
        auto expr = parsePrimary();
        if (!expr) {
            return nullptr;
        }

        while (match(TokenType::LeftParen)) {
            auto call = std::make_unique<ast::Expr>();
            call->kind = ast::ExprKind::Call;
            call->children.push_back(std::move(expr));

            if (!check(TokenType::RightParen)) {
                do {
                    auto argument = parseExpression();
                    if (!argument) {
                        return nullptr;
                    }
                    call->children.push_back(std::move(argument));
                } while (match(TokenType::Comma));
            }

            if (!match(TokenType::RightParen)) {
                return errorExprAtCurrent("Expected ')' after arguments.");
            }

            expr = std::move(call);
        }

        return expr;
    }

    std::unique_ptr<ast::Expr> parsePrimary() {
        if (match(TokenType::Number)) {
            auto expr = std::make_unique<ast::Expr>();
            expr->kind = ast::ExprKind::NumberLiteral;
            expr->value = previous().lexeme;
            return expr;
        }

        if (match(TokenType::String)) {
            auto expr = std::make_unique<ast::Expr>();
            expr->kind = ast::ExprKind::StringLiteral;
            expr->value = previous().lexeme;
            return expr;
        }

        if (match(TokenType::Identifier)) {
            auto expr = std::make_unique<ast::Expr>();
            expr->kind = ast::ExprKind::Identifier;
            expr->value = previous().lexeme;
            return expr;
        }

        if (match(TokenType::LeftParen)) {
            auto inner = parseExpression();
            if (!inner) {
                return nullptr;
            }
            if (!match(TokenType::RightParen)) {
                return errorExprAtCurrent("Expected ')' after grouped expression.");
            }

            auto grouping = std::make_unique<ast::Expr>();
            grouping->kind = ast::ExprKind::Grouping;
            grouping->children.push_back(std::move(inner));
            return grouping;
        }

        return errorExprAtCurrent("Expected expression.");
    }

    bool consumeTerminator(const std::string& message) {
        if (match(TokenType::Semicolon) || match(TokenType::Newline) || check(TokenType::RightBrace) || isAtEnd()) {
            return true;
        }
        errorAtCurrent(message);
        return false;
    }

    std::optional<ast::Statement> errorAtCurrent(const std::string& message) {
        const Token& token = peek();
        lastError_ = ParseError{token.line, token.column, message, makeHint(message)};
        return std::nullopt;
    }

    std::unique_ptr<ast::Expr> errorExprAtCurrent(const std::string& message) {
        const Token& token = peek();
        lastError_ = ParseError{token.line, token.column, message, makeHint(message)};
        return nullptr;
    }

    void synchronize() {
        while (!isAtEnd()) {
            if (current_ > 0 &&
                (previous().type == TokenType::Semicolon || previous().type == TokenType::Newline)) {
                return;
            }

            if (checkKeyword("import") || checkKeyword("use") || checkKeyword("fn") || checkKeyword("print") || checkKeyword("return")) {
                return;
            }

            advance();
        }
    }
};

} // namespace

ParseResult Parser::parse(const std::vector<lexer::Token>& tokens) const {
    ParserImpl parser(tokens);
    return parser.parseProgram();
}

std::string normalizeLine(const std::string& line) {
    return magphos::utils::trim(line);
}


std::string renderError(const ParseError& error, const std::string& source) {
    std::istringstream in(source);
    std::string lineText;
    std::size_t currentLine = 1;
    while (currentLine <= error.line && std::getline(in, lineText)) {
        if (currentLine == error.line) {
            break;
        }
        ++currentLine;
    }

    if (lineText.empty()) {
        lineText = "<line unavailable>";
    }

    const std::size_t caretColumn = error.column > 0 ? error.column - 1 : 0;
    std::string caret(caretColumn, ' ');
    caret += '^';

    std::ostringstream out;
    out << "Error at line " << error.line << ", column " << error.column << ":\n";
    out << "  " << lineText << "\n";
    out << "  " << caret << "\n";
    out << error.message << "\n";
    if (!error.hint.empty()) {
        out << "Hint: " << error.hint;
    }
    return out.str();
}

std::string renderErrors(const std::vector<ParseError>& errors, const std::string& source) {
    if (errors.empty()) {
        return "";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < errors.size(); ++i) {
        if (i > 0) {
            out << "\n\n";
        }
        out << renderError(errors[i], source);
    }
    return out.str();
}

} // namespace magphos::parser
