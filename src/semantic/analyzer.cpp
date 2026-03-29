#include "semantic/analyzer.h"

#include <sstream>
#include <unordered_set>
#include <utility>
#include <vector>

namespace magphos::semantic {

namespace {

struct Scope {
    std::unordered_set<std::string> symbols;
};

class Analyzer {
  public:
    Analyzer() {
        pushScope();
        for (const auto* name : {"len", "type", "toString", "random", "time", "sin", "cos", "sqrt", "abs", "tan", "asin", "acos", "atan", "log", "ln", "exp", "pow", "floor", "ceil", "round", "split", "replace", "substring", "join", "regexMatch", "regexReplace", "push", "pop", "map", "filter", "readFile", "writeFile", "appendFile", "read", "write", "append", "fileExists", "canvasCreate", "inputIsKeyDown", "timerNowMs", "spriteLoad", "spriteDraw", "audioPlay", "httpGet", "objectCreate", "objectSet", "objectGet", "classCreate", "env", "exec", "threadSpawn", "threadAwait", "mutexCreate", "mutexLock", "mutexUnlock", "semaphoreCreate", "semaphoreAcquire", "semaphoreRelease", "channelCreate", "channelSend", "channelRecv", "tcpConnect", "socketSend", "socketRecv", "socketClose"}) {
            scopes_.back().symbols.insert(name);
        }
    }

    std::vector<SemanticIssue> run(const ast::Program& program) {
        for (const auto& statement : program.statements) {
            analyzeStatement(statement);
        }
        return issues_;
    }

  private:
    std::vector<Scope> scopes_;
    std::vector<SemanticIssue> issues_;
    int functionDepth_ = 0;

    void pushScope() { scopes_.push_back(Scope{}); }
    void popScope() { scopes_.pop_back(); }

    void declare(const std::string& name) {
        if (!name.empty()) {
            if (scopes_.back().symbols.find(name) != scopes_.back().symbols.end()) {
                issues_.push_back({"Duplicate declaration in same scope: " + name});
                return;
            }
            scopes_.back().symbols.insert(name);
        }
    }

    bool isDefined(const std::string& name) const {
        for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
            if (it->symbols.find(name) != it->symbols.end()) {
                return true;
            }
        }
        return false;
    }

    void requireDefinedForMutation(const std::string& name, const std::string& source) {
        if (!isDefined(name)) {
            issues_.push_back({source + " requires an existing variable: " + name});
        }
    }

    void analyzeBlock(const std::vector<ast::Statement>& statements) {
        pushScope();
        for (const auto& statement : statements) {
            analyzeStatement(statement);
        }
        popScope();
    }

    void analyzeStatement(const ast::Statement& statement) {
        switch (statement.kind) {
            case ast::StmtKind::Import:
            case ast::StmtKind::Use:
                return;
            case ast::StmtKind::Function:
                declare(statement.name);
                pushScope();
                ++functionDepth_;
                for (const auto& param : statement.params) {
                    declare(param);
                }
                if (!statement.variadicParam.empty()) {
                    declare(statement.variadicParam);
                }
                for (const auto& defaultExpr : statement.paramDefaults) {
                    if (defaultExpr) {
                        analyzeExpr(*defaultExpr);
                    }
                }
                for (const auto& inner : statement.body) {
                    analyzeStatement(inner);
                }
                --functionDepth_;
                popScope();
                return;
            case ast::StmtKind::Variable:
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                declare(statement.name);
                return;
            case ast::StmtKind::Assignment:
                requireDefinedForMutation(statement.name, "Assignment");
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                return;
            case ast::StmtKind::Set:
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                requireDefinedForMutation(statement.name, "'set'");
                return;
            case ast::StmtKind::Ask:
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                requireDefinedForMutation(statement.name, "'ask'");
                return;
            case ast::StmtKind::Return:
                if (functionDepth_ <= 0) {
                    issues_.push_back({"Invalid control flow: 'return' is only allowed inside functions"});
                }
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                return;
            case ast::StmtKind::Expression:
            case ast::StmtKind::Print:
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                return;
            case ast::StmtKind::Block:
                analyzeBlock(statement.body);
                return;
            case ast::StmtKind::If:
                if (statement.condition) {
                    analyzeExpr(*statement.condition);
                }
                analyzeBlock(statement.body);
                if (!statement.elseBody.empty()) {
                    analyzeBlock(statement.elseBody);
                }
                return;
            case ast::StmtKind::When:
                if (statement.condition) {
                    analyzeExpr(*statement.condition);
                }
                analyzeBlock(statement.body);
                return;
            case ast::StmtKind::While:
            case ast::StmtKind::RepeatWhile:
                if (statement.condition) {
                    analyzeExpr(*statement.condition);
                }
                analyzeBlock(statement.body);
                return;
            case ast::StmtKind::Loop:
                if (statement.expression) {
                    analyzeExpr(*statement.expression);
                }
                analyzeBlock(statement.body);
                return;
            case ast::StmtKind::TryCatch:
                analyzeBlock(statement.body);
                analyzeBlock(statement.elseBody);
                return;
            case ast::StmtKind::Namespace:
                analyzeBlock(statement.body);
                return;
            case ast::StmtKind::Switch:
            case ast::StmtKind::Match:
                if (statement.condition) {
                    analyzeExpr(*statement.condition);
                }
                for (const auto& caseExpr : statement.caseConditions) {
                    if (caseExpr) {
                        analyzeExpr(*caseExpr);
                    }
                }
                for (const auto& caseBody : statement.caseBodies) {
                    analyzeBlock(caseBody);
                }
                if (!statement.elseBody.empty()) {
                    analyzeBlock(statement.elseBody);
                }
                return;
            case ast::StmtKind::For:
                pushScope();
                if (statement.initializer) {
                    analyzeStatement(*statement.initializer);
                }
                if (statement.condition) {
                    analyzeExpr(*statement.condition);
                }
                if (statement.increment) {
                    analyzeStatement(*statement.increment);
                }
                for (const auto& inner : statement.body) {
                    analyzeStatement(inner);
                }
                popScope();
                return;
        }
    }

    void analyzeExpr(const ast::Expr& expr) {
        if (expr.kind == ast::ExprKind::Identifier) {
            if (expr.value != "true" && expr.value != "false" && expr.value != "null" && !isDefined(expr.value)) {
                issues_.push_back({"Undefined identifier: " + expr.value});
            }
            return;
        }
        for (const auto& child : expr.children) {
            if (child) {
                analyzeExpr(*child);
            }
        }
    }
};

} // namespace

std::vector<SemanticIssue> analyze(const ast::Program& program) {
    Analyzer analyzer;
    return analyzer.run(program);
}

std::string renderIssues(const std::vector<SemanticIssue>& issues) {
    if (issues.empty()) {
        return "";
    }

    std::ostringstream out;
    for (std::size_t i = 0; i < issues.size(); ++i) {
        if (i > 0) {
            out << '\n';
        }
        out << "Semantic error: " << issues[i].message;
    }
    return out.str();
}

} // namespace magphos::semantic
