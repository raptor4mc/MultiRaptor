#include <cassert>
#include <string>

#include "interpreter/interpreter.h"

int main() {
    const std::string ok = magphos::interpreter::analyzeProgram("print (1 + 2) * 3\n");
    assert(ok == "ok");

    const std::string bad = magphos::interpreter::analyzeProgram("fn broken(a {\n");
    assert(bad.rfind("errors=", 0) == 0);
    return 0;
}
