#include "parser/parser.h"
int main() { return magphos::parser::normalizeLine("  x ") == "x" ? 0 : 1; }
