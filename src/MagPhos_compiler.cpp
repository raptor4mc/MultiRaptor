#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "multiraptor.h"
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

} // namespace

int main(int argc, char** argv) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: MagPhos_compiler <input.mp> [output.js]\n";
        return 1;
    }

    try {
        const std::string source = readFile(argv[1]);
        const std::string js = magphos::compileToJavaScript(source);

        if (argc == 3) {
            writeFile(argv[2], js);
            std::cout << "Compiled to " << argv[2] << "\n";
        } else {
            std::cout << js;
        }
    } catch (const std::exception& ex) {
        std::cerr << magphos::utils::formatError(ex.what()) << "\n";
        return 2;
    }

    return 0;
}
