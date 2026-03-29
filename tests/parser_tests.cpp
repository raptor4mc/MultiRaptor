#include <cassert>
#include <string>

#include "ast/nodes.h"
#include "lexer/lexer.h"
#include "parser/parser.h"

int main() {
    const std::string source = R"(
import math
import game.engine
use "utils.mp"

fn add(a, b) {
  return a + b
}

if true and not false {
  var cond = 1
} else {
  var cond = 2
}

while x < 10 {
  x = x + 1
}

for (var i = 0; i < 3; i = i + 1) {
  print i
}

when x > 1 {
  print x
}

loop 2 {
  print "tick"
}

repeat while x < 20 {
  set x = x + 1
}

ask "name?" -> player

var arr = [1, 2, 3]

var x = (a + b) * c;
print (x + y) / 2
)";

    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(source);

    magphos::parser::Parser parser;
    const auto result = parser.parse(tokens);

    assert(result.errors.empty());
    assert(result.program.statements.size() >= 13);
    assert(result.program.statements[0].kind == magphos::ast::StmtKind::Import);
    assert(result.program.statements[0].name == "math");
    assert(result.program.statements[1].name == "game.engine");
    assert(result.program.statements[2].kind == magphos::ast::StmtKind::Use);
    assert(result.program.statements[2].name == "utils.mp");
    assert(result.program.statements[4].kind == magphos::ast::StmtKind::If);
    assert(result.program.statements[5].kind == magphos::ast::StmtKind::While);
    assert(result.program.statements[6].kind == magphos::ast::StmtKind::For);
    assert(result.program.statements[7].kind == magphos::ast::StmtKind::When);
    assert(result.program.statements[8].kind == magphos::ast::StmtKind::Loop);
    assert(result.program.statements[9].kind == magphos::ast::StmtKind::RepeatWhile);
    assert(result.program.statements[10].kind == magphos::ast::StmtKind::Ask);
    return 0;
}
