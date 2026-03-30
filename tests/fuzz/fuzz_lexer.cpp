#include <cstdint>
#include <string>

#include "compiler/lexer/lexer.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    std::string input(reinterpret_cast<const char*>(data), size);
    magphos::lexer::Lexer lexer;
    (void)lexer.tokenize(input);
    return 0;
}
