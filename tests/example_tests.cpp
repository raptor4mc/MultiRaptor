#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "parser/parser.h"

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}
}

int main() {
    const std::vector<std::string> examples = {
        "examples/hello_world.mp",
        "examples/loops.mp",
        "examples/functions.mp",
    };

    magphos::lexer::Lexer lexer;
    magphos::parser::Parser parser;

    for (const auto& file : examples) {
        const std::string source = readFile(file);
        const auto result = parser.parse(lexer.tokenize(source));
        assert(result.errors.empty());
    }

    return 0;
}
