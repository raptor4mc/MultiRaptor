#include <cstdint>
#include <string>

#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);
    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(input);
    magphos::parser::Parser parser;
    (void)parser.parse(tokens);
    return 0;
}
