#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "compiler/ast/nodes.h"
#include "runtime/engine/environment.h"
#include "runtime/stdlib/stdlib.h"
#include "runtime/engine/value.h"

namespace magphos::runtime {

class RuntimeEngine {
  public:
    RuntimeEngine();

    void loadProgram(const ast::Program& program);
    Value evaluateExpression(const ast::Expr& expr);

    std::shared_ptr<Environment> globals() const;

  private:
    struct ReturnSignal {
        Value value;
    };
    struct StopSignal {};
    struct NextSignal {};

    StandardLibrary stdlib_;
    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> current_;
    std::unordered_map<std::string, const ast::Statement*> userFunctions_;
    std::unordered_map<std::string, std::vector<Value>> timelineHistory_;
    int loopDepth_ = 0;

    void executeStatement(const ast::Statement& statement);
    void executeBlock(const std::vector<ast::Statement>& statements, std::shared_ptr<Environment> scope);

    Value callFunctionByName(const std::string& name, const std::vector<Value>& args);
    Value evalBinary(const std::string& op, const Value& left, const Value& right);
    Value evalUnary(const std::string& op, const Value& value);
    bool isTruthy(const Value& value) const;
    bool valuesEqual(const Value& left, const Value& right) const;
};

} // namespace magphos::runtime
