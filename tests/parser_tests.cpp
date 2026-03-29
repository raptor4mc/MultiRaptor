#include <cassert>
#include <string>

#include "compiler/ast/nodes.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

int main() {
    const std::string source = R"(
import math
import game.engine
use "utils.mp"

fn add(a, b) {
  return a + b
}

fn greet(name = "friend", ...rest) {
  return name
}

namespace game {
  public fn ping() {
    return 1
  }
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

try {
  print "safe"
} catch {
  print "oops"
}

switch x {
  case 1 {
    print "one"
  }
  default {
    print "other"
  }
}

match x {
  case 2 {
    print "two"
  }
  default {
    print "other2"
  }
}

var arr = [1, 2, 3]

var x = (a + b) * c;
print (x + y) / 2
)";

    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(source);

    magphos::parser::Parser parser;
    const auto result = parser.parse(tokens);

    assert(result.errors.empty());
    assert(result.program.statements.size() >= 17);
    assert(result.program.statements[0].kind == magphos::ast::StmtKind::Import);
    assert(result.program.statements[0].name == "math");
    assert(result.program.statements[1].name == "game.engine");
    assert(result.program.statements[2].kind == magphos::ast::StmtKind::Use);
    assert(result.program.statements[2].name == "utils.mp");

    bool sawTry = false;
    bool sawSwitch = false;
    bool sawMatch = false;
    bool sawWhen = false;
    bool sawLoop = false;
    bool sawRepeatWhile = false;
    bool sawAsk = false;
    bool sawNamespace = false;
    for (const auto& statement : result.program.statements) {
        if (statement.kind == magphos::ast::StmtKind::TryCatch) sawTry = true;
        if (statement.kind == magphos::ast::StmtKind::Switch) sawSwitch = true;
        if (statement.kind == magphos::ast::StmtKind::Match) sawMatch = true;
        if (statement.kind == magphos::ast::StmtKind::When) sawWhen = true;
        if (statement.kind == magphos::ast::StmtKind::Loop) sawLoop = true;
        if (statement.kind == magphos::ast::StmtKind::RepeatWhile) sawRepeatWhile = true;
        if (statement.kind == magphos::ast::StmtKind::Ask) sawAsk = true;
        if (statement.kind == magphos::ast::StmtKind::Namespace) sawNamespace = true;
    }
    assert(sawTry);
    assert(sawSwitch);
    assert(sawMatch);
    assert(sawWhen);
    assert(sawLoop);
    assert(sawRepeatWhile);
    assert(sawAsk);
    assert(sawNamespace);
    return 0;
}
