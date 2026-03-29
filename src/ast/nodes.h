#pragma once

#include <memory>
#include <string>
#include <vector>

namespace magphos::ast {

enum class ExprKind {
    NumberLiteral,
    StringLiteral,
    BooleanLiteral,
    NullLiteral,
    Identifier,
    Unary,
    Binary,
    Grouping,
    Call,
    ArrayLiteral,
};

enum class StmtKind {
    Expression,
    Variable,
    Assignment,
    Print,
    Return,
    Function,
    Block,
    If,
    While,
    For,
    Import,
    Use,
    Set,
    Ask,
    When,
    Loop,
    RepeatWhile,
    TryCatch,
    Switch,
    Match,
    Namespace,
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
    std::vector<std::unique_ptr<Expr>> paramDefaults;
    std::string variadicParam;
    std::unique_ptr<Expr> expression;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Statement> increment;
    std::vector<Statement> body;
    std::vector<Statement> elseBody;
    std::vector<std::unique_ptr<Expr>> caseConditions;
    std::vector<std::vector<Statement>> caseBodies;
};

struct Program {
    std::vector<Statement> statements;
};

struct Node {
    std::string kind;
};

Node makeProgramNode();

} // namespace magphos::ast
