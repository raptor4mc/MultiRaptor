#include <cassert>
#include <cstdlib>
#include <functional>
#include <string>

#include "compiler/ast/nodes.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

int main() {
#if defined(_WIN32)
    _putenv_s("MAGPHOS_ENABLE_EXPERIMENTAL", "1");
#else
    setenv("MAGPHOS_ENABLE_EXPERIMENTAL", "1", 1);
#endif

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

timeline hp = 100
mood diagnostics = "mentor"

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
  next
}

repeat while x < 20 {
  set x = x + 1
  if x == 5 {
    stop
  }
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

because score from "trusted.sensor.v2" {
  print score
} else {
  print "blocked"
}

whatif world {
  print "simulate"
} compare {
  print "compare"
} commit_if (true)

match all parse("1/2/03") {
  case date_us(x) => print x
  case date_eu(x) => print x
}

negotiate runtime {
  require "deterministic_rng"
  prefer "simd128"
  fallback "portable_scalar"
}

var arr = [1, 2, 3]
print hp@now

var x = (a + b) * c;
print (x + y) / 2
)";

    magphos::lexer::Lexer lexer;
    const auto tokens = lexer.tokenize(source);

    magphos::parser::Parser parser;
    const auto result = parser.parse(tokens);

    assert(result.errors.empty());
    assert(result.program.statements.size() >= 22);
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
    bool sawTimeline = false;
    bool sawBecause = false;
    bool sawWhatIf = false;
    bool sawMood = false;
    bool sawMatchAll = false;
    bool sawNegotiate = false;
    bool sawStop = false;
    bool sawNext = false;
    std::function<void(const magphos::ast::Statement&)> scanStatement;
    scanStatement = [&](const magphos::ast::Statement& statement) {
        if (statement.kind == magphos::ast::StmtKind::TryCatch) sawTry = true;
        if (statement.kind == magphos::ast::StmtKind::Switch) sawSwitch = true;
        if (statement.kind == magphos::ast::StmtKind::Match) sawMatch = true;
        if (statement.kind == magphos::ast::StmtKind::When) sawWhen = true;
        if (statement.kind == magphos::ast::StmtKind::Loop) sawLoop = true;
        if (statement.kind == magphos::ast::StmtKind::RepeatWhile) sawRepeatWhile = true;
        if (statement.kind == magphos::ast::StmtKind::Ask) sawAsk = true;
        if (statement.kind == magphos::ast::StmtKind::Namespace) sawNamespace = true;
        if (statement.kind == magphos::ast::StmtKind::Timeline) sawTimeline = true;
        if (statement.kind == magphos::ast::StmtKind::Because) sawBecause = true;
        if (statement.kind == magphos::ast::StmtKind::WhatIf) sawWhatIf = true;
        if (statement.kind == magphos::ast::StmtKind::Mood) sawMood = true;
        if (statement.kind == magphos::ast::StmtKind::MatchAll) sawMatchAll = true;
        if (statement.kind == magphos::ast::StmtKind::Negotiate) sawNegotiate = true;
        if (statement.kind == magphos::ast::StmtKind::Stop) sawStop = true;
        if (statement.kind == magphos::ast::StmtKind::Next) sawNext = true;
        for (const auto& inner : statement.body) {
            scanStatement(inner);
        }
        for (const auto& inner : statement.elseBody) {
            scanStatement(inner);
        }
        for (const auto& caseBody : statement.caseBodies) {
            for (const auto& inner : caseBody) {
                scanStatement(inner);
            }
        }
    };
    for (const auto& statement : result.program.statements) {
        scanStatement(statement);
    }
    assert(sawTry);
    assert(sawSwitch);
    assert(sawMatch);
    assert(sawWhen);
    assert(sawLoop);
    assert(sawRepeatWhile);
    assert(sawAsk);
    assert(sawNamespace);
    assert(sawTimeline);
    assert(sawBecause);
    assert(sawWhatIf);
    assert(sawMood);
    assert(sawMatchAll);
    assert(sawNegotiate);
    assert(sawStop);
    assert(sawNext);
    return 0;
}
