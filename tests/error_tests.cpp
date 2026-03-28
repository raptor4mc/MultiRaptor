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

    return 0;
}
