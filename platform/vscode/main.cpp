#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "compiler/lexer/lexer.h"
#include "multiraptor.h"
#include "compiler/parser/parser.h"
#include "runtime/engine/engine.h"
#include "runtime/engine/errors.h"
#include "utils/error.h"

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open input file: " + path);
    }
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

void writeFile(const std::string& path, const std::string& data) {
    std::ofstream out(path);
    if (!out) {
        throw std::runtime_error("Could not write output file: " + path);
    }
    out << data;
}

void runCompilerMode(int argc, char** argv) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: magphos_vscode --compile <input.mp> [output.js]\n";
        throw std::runtime_error("Invalid arguments for compile mode.");
    }

    const std::string source = readFile(argv[2]);
    const std::string js = magphos::compileToJavaScript(source);

    if (argc == 4) {
        writeFile(argv[3], js);
        std::cout << "Compiled to " << argv[3] << "\n";
        return;
    }

    std::cout << js;
}

void runCliMode(int argc, char** argv) {
    if (argc < 3) {
        std::cout << "MagPhos VSCode CLI mode\n";
        std::cout << "Usage:\n";
        std::cout << "  magphos_vscode --cli --version\n";
        std::cout << "  magphos_vscode --cli --about\n";
        return;
    }

    const std::string cmd = argv[2];
    if (cmd == "--version") {
        std::cout << "MagPhos CLI v0.1.0\n";
        return;
    }
    if (cmd == "--about") {
        std::cout << "MagPhos includes a native lexer/parser/runtime toolchain.\n";
        return;
    }

    std::cerr << "Unknown --cli command: " << cmd << "\n";
    throw std::runtime_error("Invalid CLI command.");
}

void runReplMode() {
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
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "MagPhos VSCode launcher\n";
        std::cout << "Activate everything from one entrypoint:\n";
        std::cout << "  magphos_vscode --compile <input.mp> [output.js]\n";
        std::cout << "  magphos_vscode --cli [--version|--about]\n";
        std::cout << "  magphos_vscode --repl\n";
        return 0;
    }

    try {
        const std::string mode = argv[1];
        if (mode == "--compile") {
            runCompilerMode(argc, argv);
            return 0;
        }
        if (mode == "--cli") {
            runCliMode(argc, argv);
            return 0;
        }
        if (mode == "--repl") {
            runReplMode();
            return 0;
        }

        std::cerr << "Unknown mode: " << mode << "\n";
        return 2;
    } catch (const std::exception& ex) {
        std::cerr << magphos::utils::formatError(ex.what()) << "\n";
        return 2;
    }
}
