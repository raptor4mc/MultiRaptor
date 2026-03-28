#include "runtime/engine.h"

#include <cmath>

namespace magphos::runtime {

RuntimeEngine::RuntimeEngine() : globals_(std::make_shared<Environment>()), current_(globals_) {}

void RuntimeEngine::loadProgram(const ast::Program& program) {
    for (const auto& statement : program.statements) {
        executeStatement(statement);
    }
}

Value RuntimeEngine::evaluateExpression(const ast::Expr& expr) {
    switch (expr.kind) {
        case ast::ExprKind::NumberLiteral:
            return Value(std::stod(expr.value));
        case ast::ExprKind::StringLiteral:
            return Value(expr.value);
        case ast::ExprKind::BooleanLiteral:
            return Value(expr.value == "true");
        case ast::ExprKind::NullLiteral:
            return Value::makeNull();
        case ast::ExprKind::Identifier:
            return current_->get(expr.value);
        case ast::ExprKind::Grouping:
            return evaluateExpression(*expr.children[0]);
        case ast::ExprKind::Unary:
            return evalUnary(expr.value, evaluateExpression(*expr.children[0]));
        case ast::ExprKind::Binary:
            return evalBinary(expr.value,
                              evaluateExpression(*expr.children[0]),
                              evaluateExpression(*expr.children[1]));
        case ast::ExprKind::Call: {
            const auto& callee = *expr.children[0];
            if (callee.kind != ast::ExprKind::Identifier) {
                throw RuntimeError(RuntimeErrorCode::TypeError,
                                   "Can only call named functions.",
                                   "Use a function name before parentheses, e.g. add(1, 2).");
            }
            std::vector<Value> args;
            for (std::size_t i = 1; i < expr.children.size(); ++i) {
                args.push_back(evaluateExpression(*expr.children[i]));
            }
            return callFunctionByName(callee.value, args);
        }
    }

    throw RuntimeError(RuntimeErrorCode::RuntimeFailure, "Unknown expression kind.");
}

std::shared_ptr<Environment> RuntimeEngine::globals() const {
    return globals_;
}

void RuntimeEngine::executeStatement(const ast::Statement& statement) {
    switch (statement.kind) {
        case ast::StmtKind::Import:
        case ast::StmtKind::Use:
            return;
        case ast::StmtKind::Function:
            userFunctions_[statement.name] = &statement;
            globals_->define(statement.name, Value::makeFunction(statement.name, statement.params));
            return;
        case ast::StmtKind::Variable: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Variable declaration missing expression.");
            }
            current_->define(statement.name, evaluateExpression(*statement.expression));
            return;
        }
        case ast::StmtKind::Assignment: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Assignment missing expression.");
            }
            const Value value = evaluateExpression(*statement.expression);
            try {
                current_->assign(statement.name, value);
            } catch (const RuntimeError&) {
                current_->define(statement.name, value);
            }
            return;
        }
        case ast::StmtKind::Expression:
        case ast::StmtKind::Print: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Statement missing expression.");
            }
            (void)evaluateExpression(*statement.expression);
            return;
        }
        case ast::StmtKind::Return: {
            if (!statement.expression) {
                throw ReturnSignal{Value::makeNull()};
            }
            throw ReturnSignal{evaluateExpression(*statement.expression)};
        }
        case ast::StmtKind::Block: {
            auto child = std::make_shared<Environment>(current_);
            executeBlock(statement.body, child);
            return;
        }
        case ast::StmtKind::If: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "If statement missing condition.");
            }
            if (isTruthy(evaluateExpression(*statement.condition))) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            } else if (!statement.elseBody.empty()) {
                executeBlock(statement.elseBody, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::While: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "While statement missing condition.");
            }
            while (isTruthy(evaluateExpression(*statement.condition))) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::For: {
            auto loopScope = std::make_shared<Environment>(current_);
            const auto previous = current_;
            current_ = loopScope;
            try {
                if (statement.initializer) {
                    executeStatement(*statement.initializer);
                }
                while (!statement.condition || isTruthy(evaluateExpression(*statement.condition))) {
                    executeBlock(statement.body, std::make_shared<Environment>(loopScope));
                    if (statement.increment) {
                        executeStatement(*statement.increment);
                    }
                }
            } catch (...) {
                current_ = previous;
                throw;
            }
            current_ = previous;
            return;
        }
    }
}

void RuntimeEngine::executeBlock(const std::vector<ast::Statement>& statements, std::shared_ptr<Environment> scope) {
    const auto previous = current_;
    current_ = std::move(scope);
    try {
        for (const auto& statement : statements) {
            executeStatement(statement);
        }
    } catch (...) {
        current_ = previous;
        throw;
    }
    current_ = previous;
}

Value RuntimeEngine::callFunctionByName(const std::string& name, const std::vector<Value>& args) {
    if (stdlib_.has(name)) {
        return stdlib_.call(name, args);
    }

    const auto it = userFunctions_.find(name);
    if (it == userFunctions_.end()) {
        throw RuntimeError(RuntimeErrorCode::NameError,
                           "Unknown function: " + name,
                           "Define the function before calling it, or check the name.");
    }

    const ast::Statement& function = *it->second;
    if (function.params.size() != args.size()) {
        throw RuntimeError(RuntimeErrorCode::ArityError,
                           "Function '" + name + "' expects " + std::to_string(function.params.size()) +
                               " arguments but got " + std::to_string(args.size()) + ".",
                           "Pass the exact number of arguments required by the function signature.");
    }

    auto callScope = std::make_shared<Environment>(globals_);
    for (std::size_t i = 0; i < args.size(); ++i) {
        callScope->define(function.params[i], args[i]);
    }

    try {
        executeBlock(function.body, callScope);
    } catch (const ReturnSignal& signal) {
        return signal.value;
    }

    return Value::makeNull();
}

Value RuntimeEngine::evalBinary(const std::string& op, const Value& left, const Value& right) {
    if (op == "and") {
        return Value(isTruthy(left) && isTruthy(right));
    }
    if (op == "or") {
        return Value(isTruthy(left) || isTruthy(right));
    }
    if (op == "==" || op == "!=") {
        const bool equal = valuesEqual(left, right);
        return Value(op == "==" ? equal : !equal);
    }
    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        if (left.type() != TypeKind::Number || right.type() != TypeKind::Number) {
            throw RuntimeError(RuntimeErrorCode::TypeError,
                               "Comparison operators require numeric operands.",
                               "Use numbers with <, <=, >, >=.");
        }
        if (op == "<") {
            return Value(left.asNumber() < right.asNumber());
        }
        if (op == "<=") {
            return Value(left.asNumber() <= right.asNumber());
        }
        if (op == ">") {
            return Value(left.asNumber() > right.asNumber());
        }
        return Value(left.asNumber() >= right.asNumber());
    }

    if (op == "+") {
        if (left.type() == TypeKind::String || right.type() == TypeKind::String) {
            return Value(stdlib_.call("toString", {left}).asString() + stdlib_.call("toString", {right}).asString());
        }
        if (left.type() == TypeKind::Number && right.type() == TypeKind::Number) {
            return Value(left.asNumber() + right.asNumber());
        }
    }

    if (left.type() != TypeKind::Number || right.type() != TypeKind::Number) {
        throw RuntimeError(RuntimeErrorCode::TypeError,
                           "Operator '" + op + "' requires numeric operands.",
                           "Convert values to numbers before applying arithmetic operators.");
    }

    if (op == "-") {
        return Value(left.asNumber() - right.asNumber());
    }
    if (op == "*") {
        return Value(left.asNumber() * right.asNumber());
    }
    if (op == "/") {
        if (std::abs(right.asNumber()) < 1e-12) {
            throw RuntimeError(RuntimeErrorCode::RuntimeFailure,
                               "Division by zero.",
                               "Ensure the denominator is not zero before dividing.");
        }
        return Value(left.asNumber() / right.asNumber());
    }

    throw RuntimeError(RuntimeErrorCode::TypeError,
                       "Unsupported operator: " + op,
                       "Use one of +, -, *, /. ");
}

Value RuntimeEngine::evalUnary(const std::string& op, const Value& value) {
    if (op == "-" ) {
        if (value.type() != TypeKind::Number) {
            throw RuntimeError(RuntimeErrorCode::TypeError,
                               "Unary '-' expects a number.",
                               "Use a numeric value, e.g. -10.");
        }
        return Value(-value.asNumber());
    }
    if (op == "!" || op == "not") {
        return Value(!isTruthy(value));
    }

    throw RuntimeError(RuntimeErrorCode::TypeError, "Unsupported unary operator: " + op);
}

bool RuntimeEngine::isTruthy(const Value& value) const {
    if (value.isNull()) {
        return false;
    }
    if (value.type() == TypeKind::Boolean) {
        return value.asBoolean();
    }
    if (value.type() == TypeKind::Number) {
        return std::abs(value.asNumber()) > 1e-12;
    }
    if (value.type() == TypeKind::String) {
        return !value.asString().empty();
    }
    return true;
}

bool RuntimeEngine::valuesEqual(const Value& left, const Value& right) const {
    if (left.type() != right.type()) {
        return false;
    }
    switch (left.type()) {
        case TypeKind::Null:
            return true;
        case TypeKind::Boolean:
            return left.asBoolean() == right.asBoolean();
        case TypeKind::Number:
            return std::abs(left.asNumber() - right.asNumber()) < 1e-12;
        case TypeKind::String:
            return left.asString() == right.asString();
        default:
            return false;
    }
}

} // namespace magphos::runtime
