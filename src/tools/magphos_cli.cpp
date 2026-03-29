#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/module_system.h"
#include "runtime/engine.h"
#include "runtime/errors.h"

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "MagPhos CLI\n";
        std::cout << "Usage:\n";
        std::cout << "  magphos_cli --version\n";
        std::cout << "  magphos_cli --about\n";
        std::cout << "  magphos_cli --check <program.mp>\n";
        std::cout << "  magphos_cli --deps <program.mp> [baseDir]\n";
        std::cout << "  magphos_cli --tokens <program.mp>\n";
        return 0;
    }

    const std::string cmd = argv[1];
    if (cmd == "--version") {
        std::cout << "MagPhos CLI v0.1.0\n";
        return 0;
    }
    if (cmd == "--about") {
        std::cout << "MagPhos includes a native lexer/parser/runtime toolchain.\n";
        return 0;
    }
    if (cmd == "--check" && argc >= 3) {
        const std::string source = readFile(argv[2]);
        if (source.empty()) {
            std::cerr << "Cannot read file: " << argv[2] << "\n";
            return 2;
        }
        const std::string result = magphos::interpreter::analyzeProgram(source);
        if (result == "ok") {
            std::cout << "ok\n";
            return 0;
        }
        std::cerr << result << "\n";
        return 3;
    }
    if (cmd == "--tokens" && argc >= 3) {
        const std::string source = readFile(argv[2]);
        if (source.empty()) {
            std::cerr << "Cannot read file: " << argv[2] << "\n";
            return 2;
        }
        magphos::lexer::Lexer lexer;
        const auto tokens = lexer.tokenize(source);
        for (const auto& token : tokens) {
            std::cout << token.line << ":" << token.column << " '" << token.lexeme << "'\n";
        }
        return 0;
    }
    if (cmd == "--deps" && argc >= 3) {
        const std::string source = readFile(argv[2]);
        if (source.empty()) {
            std::cerr << "Cannot read file: " << argv[2] << "\n";
            return 2;
        }
        magphos::lexer::Lexer lexer;
        magphos::parser::Parser parser;
        const auto parsed = parser.parse(lexer.tokenize(source));
        if (!parsed.errors.empty()) {
            std::cerr << magphos::parser::renderErrors(parsed.errors, source) << "\n";
            return 3;
        }
        magphos::runtime::ModuleSystem modules;
        const auto deps = modules.collectDependencies(parsed);
        for (const auto& dep : deps) {
            std::cout << dep << "\n";
        }
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    return 2;
}
