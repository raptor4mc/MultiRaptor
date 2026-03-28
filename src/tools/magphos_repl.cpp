#include <iostream>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/engine.h"
#include "runtime/errors.h"

int main() {
    magphos::runtime::RuntimeEngine engine;
    magphos::lexer::Lexer lexer;
    magphos::parser::Parser parser;

    std::cout << "MagPhos REPL (type :quit to exit)\n";
    for (;;) {
        std::cout << "mp> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line == ":quit" || line == ":exit") {
            break;
        }
        if (line.empty()) {
            continue;
        }

        const auto result = parser.parse(lexer.tokenize(line + "\n"));
        if (!result.errors.empty()) {
            std::cout << magphos::parser::renderErrors(result.errors, line + "\n") << "\n";
            continue;
        }

        try {
            engine.loadProgram(result.program);
            std::cout << "ok\n";
        } catch (const magphos::runtime::RuntimeError& ex) {
            std::cout << "Runtime error: " << ex.what();
            if (!ex.hint().empty()) {
                std::cout << "\nHint: " << ex.hint();
            }
            std::cout << "\n";
        }
    }

    return 0;
}
