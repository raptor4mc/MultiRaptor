#include <iostream>
#include <string>

#include "runtime/engine.h"
#include "runtime/errors.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "MagPhos CLI\n";
        std::cout << "Usage:\n";
        std::cout << "  magphos_cli --version\n";
        std::cout << "  magphos_cli --about\n";
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

    std::cerr << "Unknown command: " << cmd << "\n";
    return 2;
}
