#include "lexer/lexer.h"
int main() { return magphos::lexer::splitWhitespace("a b").size() == 2 ? 0 : 1; }
