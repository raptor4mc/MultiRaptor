#pragma once

#include <memory>
#include <string>
#include <vector>

namespace magphos::ast {

enum class ExprKind {
    NumberLiteral,
    StringLiteral,
    Identifier,
    Unary,
    Binary,
    Grouping,
    Call,
};

enum class StmtKind {
    Expression,
    Assignment,
    Print,
    Return,
    Function,
    Block,
};

struct Expr {
    ExprKind kind;
    std::string value;
    std::vector<std::unique_ptr<Expr>> children;
};

struct Statement {
    StmtKind kind;
    std::string name;
    std::vector<std::string> params;
    std::unique_ptr<Expr> expression;
    std::vector<Statement> body;
};

struct Program {
    std::vector<Statement> statements;
};

struct Node {
    std::string kind;
};

Node makeProgramNode();

} // namespace magphos::ast
