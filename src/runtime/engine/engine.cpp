#include "runtime/engine/engine.h"

#include <cmath>
#include <iostream>

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
        case ast::ExprKind::ArrayLiteral: {
            ArrayValue array;
            for (const auto& child : expr.children) {
                array.elements.push_back(std::make_shared<Value>(evaluateExpression(*child)));
            }
            return Value::makeArray(array);
        }
        case ast::ExprKind::TimelineAccess: {
            if (expr.children.size() < 2 || expr.children[0]->kind != ast::ExprKind::Identifier) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Timeline access expects identifier@index.");
            }
            const std::string& timelineName = expr.children[0]->value;
            const auto it = timelineHistory_.find(timelineName);
            if (it == timelineHistory_.end() || it->second.empty()) {
                throw RuntimeError(RuntimeErrorCode::NameError, "Unknown timeline variable: " + timelineName);
            }
            if (expr.children[1]->kind == ast::ExprKind::Identifier && expr.children[1]->value == "now") {
                return it->second.back();
            }
            if (expr.children[1]->kind != ast::ExprKind::NumberLiteral) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Timeline index must be numeric or 'now'.");
            }
            const std::size_t idx = static_cast<std::size_t>(std::stoll(expr.children[1]->value));
            if (idx >= it->second.size()) {
                throw RuntimeError(RuntimeErrorCode::RuntimeFailure, "Timeline index out of range.");
            }
            return it->second[idx];
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
        case ast::StmtKind::Timeline: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Timeline declaration missing expression.");
            }
            const Value initial = evaluateExpression(*statement.expression);
            current_->define(statement.name, initial);
            timelineHistory_[statement.name] = {initial};
            return;
        }
        case ast::StmtKind::Assignment: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Assignment missing expression.");
            }
            const Value value = evaluateExpression(*statement.expression);
            current_->assign(statement.name, value);
            const auto history = timelineHistory_.find(statement.name);
            if (history != timelineHistory_.end()) {
                history->second.push_back(value);
            }
            return;
        }
        case ast::StmtKind::Expression: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Statement missing expression.");
            }
            (void)evaluateExpression(*statement.expression);
            return;
        }
        case ast::StmtKind::Print: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Print statement missing expression.");
            }
            const Value printed = evaluateExpression(*statement.expression);
            std::cout << stdlib_.call("toString", {printed}).asString() << "\n";
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
        case ast::StmtKind::Set: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Set statement missing expression.");
            }
            const Value value = evaluateExpression(*statement.expression);
            current_->assign(statement.name, value);
            const auto history = timelineHistory_.find(statement.name);
            if (history != timelineHistory_.end()) {
                history->second.push_back(value);
            }
            return;
        }
        case ast::StmtKind::Ask: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Ask statement missing prompt expression.");
            }
            const Value prompt = evaluateExpression(*statement.expression);
            std::cout << stdlib_.call("toString", {prompt}).asString();
            std::string input;
            std::getline(std::cin, input);
            current_->assign(statement.name, Value(input));
            return;
        }
        case ast::StmtKind::When: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "When statement missing condition.");
            }
            if (isTruthy(evaluateExpression(*statement.condition))) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::Loop: {
            if (!statement.expression) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Loop statement missing count expression.");
            }
            const Value countValue = evaluateExpression(*statement.expression);
            if (countValue.type() != TypeKind::Number) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Loop count must evaluate to a number.");
            }
            const long long count = static_cast<long long>(countValue.asNumber());
            for (long long i = 0; i < count; ++i) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::RepeatWhile: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Repeat while statement missing condition.");
            }
            do {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            } while (isTruthy(evaluateExpression(*statement.condition)));
            return;
        }
        case ast::StmtKind::TryCatch: {
            try {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            } catch (const RuntimeError&) {
                executeBlock(statement.elseBody, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::Switch:
        case ast::StmtKind::Match: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Switch/match statement missing condition.");
            }
            const Value key = evaluateExpression(*statement.condition);
            for (std::size_t i = 0; i < statement.caseConditions.size(); ++i) {
                const Value caseValue = evaluateExpression(*statement.caseConditions[i]);
                if (valuesEqual(key, caseValue)) {
                    executeBlock(statement.caseBodies[i], std::make_shared<Environment>(current_));
                    return;
                }
            }
            if (!statement.elseBody.empty()) {
                executeBlock(statement.elseBody, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::Namespace: {
            executeBlock(statement.body, std::make_shared<Environment>(current_));
            return;
        }
        case ast::StmtKind::Because: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Because statement missing guarded expression.");
            }
            if (isTruthy(evaluateExpression(*statement.condition))) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            } else if (!statement.elseBody.empty()) {
                executeBlock(statement.elseBody, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::WhatIf: {
            if (statement.condition && isTruthy(evaluateExpression(*statement.condition))) {
                executeBlock(statement.body, std::make_shared<Environment>(current_));
            } else {
                executeBlock(statement.elseBody, std::make_shared<Environment>(current_));
            }
            return;
        }
        case ast::StmtKind::Mood:
            return;
        case ast::StmtKind::MatchAll: {
            if (!statement.condition) {
                throw RuntimeError(RuntimeErrorCode::TypeError, "Match all statement missing condition.");
            }
            const Value key = evaluateExpression(*statement.condition);
            for (std::size_t i = 0; i < statement.caseConditions.size(); ++i) {
                const Value caseValue = evaluateExpression(*statement.caseConditions[i]);
                if (valuesEqual(key, caseValue) && !statement.caseBodies[i].empty()) {
                    executeStatement(statement.caseBodies[i][0]);
                }
            }
            return;
        }
        case ast::StmtKind::Negotiate:
            return;
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
        try {
            return stdlib_.call(name, args);
        } catch (const RuntimeError&) {
            throw;
        } catch (const std::exception& ex) {
            throw RuntimeError(RuntimeErrorCode::RuntimeFailure,
                               "Builtin '" + name + "' failed: " + ex.what(),
                               "Check argument types/count and external system availability.");
        }
    }

    const auto it = userFunctions_.find(name);
    if (it == userFunctions_.end()) {
        throw RuntimeError(RuntimeErrorCode::NameError,
                           "Unknown function: " + name,
                           "Define the function before calling it, or check the name.");
    }

    const ast::Statement& function = *it->second;
    std::size_t requiredParams = 0;
    for (std::size_t i = 0; i < function.params.size(); ++i) {
        if (i >= function.paramDefaults.size() || !function.paramDefaults[i]) {
            ++requiredParams;
        }
    }

    if (args.size() < requiredParams || (function.variadicParam.empty() && args.size() > function.params.size())) {
        throw RuntimeError(RuntimeErrorCode::ArityError,
                           "Function '" + name + "' expects at least " + std::to_string(requiredParams) +
                               " arguments" + (function.variadicParam.empty() ? "" : " (variadic enabled)") +
                               " but got " + std::to_string(args.size()) + ".",
                           "Pass required arguments and use optional/default parameters correctly.");
    }

    auto callScope = std::make_shared<Environment>(globals_);
    for (std::size_t i = 0; i < function.params.size(); ++i) {
        if (i < args.size()) {
            callScope->define(function.params[i], args[i]);
        } else if (i < function.paramDefaults.size() && function.paramDefaults[i]) {
            const auto prev = current_;
            current_ = callScope;
            const Value defaultValue = evaluateExpression(*function.paramDefaults[i]);
            current_ = prev;
            callScope->define(function.params[i], defaultValue);
        }
    }

    if (!function.variadicParam.empty()) {
        ArrayValue vargs;
        for (std::size_t i = function.params.size(); i < args.size(); ++i) {
            vargs.elements.push_back(std::make_shared<Value>(args[i]));
        }
        callScope->define(function.variadicParam, Value::makeArray(vargs));
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
