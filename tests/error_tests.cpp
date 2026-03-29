#include <cassert>
#include <string>

#include "interpreter/interpreter.h"

int main() {
    const std::string missingExpr = magphos::interpreter::analyzeProgram("x =\n");
    assert(missingExpr.find("Expected expression") != std::string::npos);
    assert(missingExpr.find("Hint:") != std::string::npos);
    assert(missingExpr.find("^") != std::string::npos);

    const std::string badUse = magphos::interpreter::analyzeProgram("use utils.mp\n");
    assert(badUse.find("Expected quoted path after 'use'") != std::string::npos);
    assert(badUse.find("use \"utils.mp\"") != std::string::npos);

    const std::string badAssign = magphos::interpreter::analyzeProgram("score = 10\n");
    assert(badAssign.find("Assignment requires an existing variable: score") != std::string::npos);

    const std::string badSet = magphos::interpreter::analyzeProgram("set score = 10\n");
    assert(badSet.find("'set' requires an existing variable: score") != std::string::npos);

    const std::string badAsk = magphos::interpreter::analyzeProgram("ask \"name?\" -> user\n");
    assert(badAsk.find("'ask' requires an existing variable: user") != std::string::npos);

    const std::string badReturn = magphos::interpreter::analyzeProgram("return 42\n");
    assert(badReturn.find("Invalid control flow: 'return' is only allowed inside functions") != std::string::npos);

    return 0;
}
