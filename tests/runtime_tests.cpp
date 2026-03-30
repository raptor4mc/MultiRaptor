#include <cassert>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include "runtime/interpreter/interpreter.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"
#include "runtime/engine/engine.h"
#include "runtime/engine/environment.h"
#include "runtime/engine/errors.h"
#include "runtime/engine/module_system.h"
#include "runtime/stdlib/stdlib.h"
#include "runtime/engine/value.h"

int main() {
    const std::string ok = magphos::interpreter::analyzeProgram("print (1 + 2) * 3\n");
    assert(ok == "ok");

    const std::string bad = magphos::interpreter::analyzeProgram("fn broken(a {\n");
    assert(bad.find("Error at line") != std::string::npos);
    assert(bad.find("^") != std::string::npos);
    assert(bad.find("Hint:") != std::string::npos);

    using magphos::runtime::ArrayValue;
    using magphos::runtime::ObjectValue;
    using magphos::runtime::RuntimeEngine;
    using magphos::runtime::RuntimeError;
    using magphos::runtime::RuntimeErrorCode;
    using magphos::runtime::StandardLibrary;
    using magphos::runtime::TypeKind;
    using magphos::runtime::Value;

    const Value numberValue(42.0);
    const Value stringValue(std::string("magphos"));
    const Value boolValue(true);
    const Value nullValue = Value::makeNull();
    const Value functionValue = Value::makeFunction("add", {"a", "b"});

    ObjectValue fields;
    fields.fields["name"] = std::make_shared<Value>(Value(std::string("core")));
    const Value objectValue = Value::makeObject(fields);

    ArrayValue list;
    list.elements.push_back(std::make_shared<Value>(Value(1.0)));
    list.elements.push_back(std::make_shared<Value>(Value(2.0)));
    const Value arrayValue = Value::makeArray(list);

    magphos::runtime::MapValue mapEntries;
    mapEntries.entries["k"] = std::make_shared<Value>(Value(10.0));
    const Value mapValue = Value::makeMap(mapEntries);

    const Value classValue = Value::makeClass("Widget", fields);
    const Value structValue = Value::makeStruct("Point", fields);
    const Value enumValue = Value::makeEnum("Color", "Red");

    assert(numberValue.type() == TypeKind::Number && numberValue.asNumber() == 42.0);
    assert(stringValue.type() == TypeKind::String && stringValue.asString() == "magphos");
    assert(boolValue.type() == TypeKind::Boolean && boolValue.asBoolean());
    assert(nullValue.type() == TypeKind::Null && nullValue.isNull());
    assert(functionValue.type() == TypeKind::Function && functionValue.asFunction().params.size() == 2);
    assert(objectValue.type() == TypeKind::Object);
    assert(arrayValue.type() == TypeKind::Array && arrayValue.asArray().elements.size() == 2);
    assert(mapValue.type() == TypeKind::Map && mapValue.asMap().entries.find("k") != mapValue.asMap().entries.end());
    assert(classValue.type() == TypeKind::Class && classValue.asClass().name == "Widget");
    assert(structValue.type() == TypeKind::Struct && structValue.asStruct().name == "Point");
    assert(enumValue.type() == TypeKind::Enum && enumValue.asEnum().variant == "Red");

    magphos::runtime::Environment env;
    env.set("answer", numberValue);
    assert(env.get("answer").asNumber() == 42.0);
    bool missingRaised = false;
    try {
        (void)env.get("missing");
    } catch (const RuntimeError& ex) {
        missingRaised = ex.code() == RuntimeErrorCode::NameError;
    }
    assert(missingRaised);

    StandardLibrary stdlib;
    assert(stdlib.call("len", {stringValue}).asNumber() == 7.0);
    assert(stdlib.call("type", {arrayValue}).asString() == "array");
    assert(stdlib.call("toString", {enumValue}).asString() == "Color.Red");
    assert(stdlib.call("random", {}).type() == TypeKind::Number);
    assert(stdlib.call("time", {}).type() == TypeKind::Number);
    assert(std::abs(stdlib.call("sin", {Value(0.0)}).asNumber()) < 1e-9);
    assert(std::abs(stdlib.call("cos", {Value(0.0)}).asNumber() - 1.0) < 1e-9);
    assert(stdlib.call("sqrt", {Value(9.0)}).asNumber() == 3.0);
    assert(stdlib.call("abs", {Value(-5.0)}).asNumber() == 5.0);
    assert(stdlib.call("pow", {Value(2.0), Value(3.0)}).asNumber() == 8.0);
    assert(stdlib.call("round", {Value(2.6)}).asNumber() == 3.0);

    const auto splitResult = stdlib.call("split", {Value(std::string("a,b,c")), Value(std::string(","))});
    assert(splitResult.type() == TypeKind::Array);
    assert(splitResult.asArray().elements.size() == 3);
    assert(stdlib.call("replace", {Value(std::string("aa")), Value(std::string("a")), Value(std::string("b"))}).asString() == "bb");
    assert(stdlib.call("substring", {Value(std::string("magphos")), Value(3.0), Value(3.0)}).asString() == "pho");
    assert(stdlib.call("join", {splitResult, Value(std::string("-"))}).asString() == "a-b-c");
    assert(stdlib.call("regexMatch", {Value(std::string("abc123")), Value(std::string("[a-z]+\\d+"))}).asBoolean());
    assert(stdlib.call("regexReplace", {Value(std::string("abc123")), Value(std::string("\\d+")), Value(std::string("X"))}).asString() == "abcX");

    const auto pushed = stdlib.call("push", {arrayValue, Value(3.0)});
    assert(pushed.asArray().elements.size() == 3);
    const auto popped = stdlib.call("pop", {pushed});
    assert(popped.asArray().elements.size() == 2);
    const auto mapped = stdlib.call("map", {arrayValue, Value(std::string("abs"))});
    assert(mapped.asArray().elements.size() == 2);
    const auto filtered = stdlib.call("filter", {pushed, Value(std::string("gt")), Value(1.5)});
    assert(filtered.asArray().elements.size() == 2);


    // Game/graphics API placeholders.
    const auto canvas = stdlib.call("canvasCreate", {Value(800.0), Value(600.0)});
    assert(canvas.type() == TypeKind::Object);
    assert(canvas.asObject().fields.find("kind") != canvas.asObject().fields.end());
    const auto sprite = stdlib.call("spriteLoad", {Value(std::string("hero.png"))});
    assert(sprite.type() == TypeKind::Object);
    assert(stdlib.call("inputIsKeyDown", {Value(std::string("Space"))}).type() == TypeKind::Boolean);
    assert(stdlib.call("timerNowMs", {}).type() == TypeKind::Number);
    assert(stdlib.call("spriteDraw", {canvas, sprite, Value(10.0), Value(20.0)}).isNull());
    assert(stdlib.call("audioPlay", {Value(std::string("theme.ogg"))}).isNull());

        const std::string path = "/tmp/magphos_stdlib_test.txt";
    stdlib.call("writeFile", {Value(path), Value(std::string("hello"))});
    assert(stdlib.call("readFile", {Value(path)}).asString() == "hello");
    stdlib.call("appendFile", {Value(path), Value(std::string(" world"))});
    assert(stdlib.call("readFile", {Value(path)}).asString() == "hello world");
    assert(stdlib.call("fileExists", {Value(path)}).asBoolean());
    stdlib.call("write", {Value(path), Value(std::string("a"))});
    stdlib.call("append", {Value(path), Value(std::string("b"))});
    assert(stdlib.call("read", {Value(path)}).asString() == "ab");

    const auto obj0 = stdlib.call("objectCreate", {});
    const auto obj1 = stdlib.call("objectSet", {obj0, Value(std::string("hp")), Value(100.0)});
    assert(stdlib.call("objectGet", {obj1, Value(std::string("hp"))}).asNumber() == 100.0);
    assert(stdlib.call("classCreate", {Value(std::string("Enemy"))}).asClass().name == "Enemy");
    assert(stdlib.call("env", {Value(std::string("PATH"))}).type() == TypeKind::String);
    assert(stdlib.call("exec", {Value(std::string("printf hi"))}).asString() == "hi");
    stdlib.call("writeFile", {Value(std::string("/tmp/magphos_httpget.txt")), Value(std::string("network-ok"))});
    assert(stdlib.call("httpGet", {Value(std::string("file:///tmp/magphos_httpget.txt"))}).asString().find("network-ok") != std::string::npos);

    const auto threadHandle = stdlib.call("threadSpawn", {Value(1.0), Value(std::string("done"))});
    assert(stdlib.call("threadAwait", {threadHandle}).asString() == "done");
    const auto mutexHandle = stdlib.call("mutexCreate", {});
    assert(stdlib.call("mutexLock", {mutexHandle}).isNull());
    assert(stdlib.call("mutexUnlock", {mutexHandle}).isNull());
    const auto semHandle = stdlib.call("semaphoreCreate", {Value(1.0)});
    assert(stdlib.call("semaphoreAcquire", {semHandle}).isNull());
    assert(stdlib.call("semaphoreRelease", {semHandle}).isNull());
    const auto channelHandle = stdlib.call("channelCreate", {});
    assert(stdlib.call("channelSend", {channelHandle, Value(std::string("msg"))}).isNull());
    assert(stdlib.call("channelRecv", {channelHandle}).asString() == "msg");

    const int testPort = 40555;
    std::thread server([&]() {
        int srv = ::socket(AF_INET, SOCK_STREAM, 0);
        assert(srv >= 0);
        int opt = 1;
        setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(testPort));
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        assert(bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
        assert(listen(srv, 1) == 0);
        int client = accept(srv, nullptr, nullptr);
        assert(client >= 0);
        char buf[16] = {0};
        const auto n = recv(client, buf, sizeof(buf), 0);
        assert(n > 0);
        send(client, "pong", 4, 0);
        close(client);
        close(srv);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const auto socketHandle = stdlib.call("tcpConnect", {Value(std::string("127.0.0.1")), Value(static_cast<double>(testPort))});
    assert(stdlib.call("socketSend", {socketHandle, Value(std::string("ping"))}).asNumber() >= 4.0);
    assert(stdlib.call("socketRecv", {socketHandle, Value(16.0)}).asString() == "pong");
    assert(stdlib.call("socketClose", {socketHandle}).isNull());
    server.join();

    magphos::runtime::ModuleSystem moduleSystem;
    const std::string modBase = "/tmp/magphos_modules";
    std::filesystem::create_directories(modBase + "/game");
    stdlib.call("writeFile", {Value(modBase + "/math.mp"), Value(std::string("print 1\n"))});
    stdlib.call("writeFile", {Value(modBase + "/game/engine.mp"), Value(std::string("print 2\n"))});
    stdlib.call("writeFile", {Value(modBase + "/utils.mp"), Value(std::string("print 3\n"))});

    assert(moduleSystem.resolveModulePath("game.engine", modBase) == modBase + "/game/engine.mp");
    assert(moduleSystem.resolveUsePath("utils.mp", modBase) == modBase + "/utils.mp");
    assert(moduleSystem.loadImportedModule("math", modBase).find("print 1") != std::string::npos);
    assert(moduleSystem.loadUsePath("utils.mp", modBase).find("print 3") != std::string::npos);
    assert(moduleSystem.loadImportedModule("math", modBase).find("print 1") != std::string::npos); // cache path

    // Cache determinism: content should remain stable until cache clear.
    stdlib.call("writeFile", {Value(modBase + "/math.mp"), Value(std::string("print 99\n"))});
    assert(moduleSystem.loadImportedModule("math", modBase).find("print 1") != std::string::npos);
    moduleSystem.clearCache();
    assert(moduleSystem.loadImportedModule("math", modBase).find("print 99") != std::string::npos);

    const std::string depSource = "import game.engine\nuse \"utils.mp\"\nprint 1\n";
    magphos::lexer::Lexer depLexer;
    magphos::parser::Parser depParser;
    const auto depResult = depParser.parse(depLexer.tokenize(depSource));
    const auto deps = moduleSystem.collectDependencies(depResult);
    assert(deps.size() == 2);
    assert(deps[0] == "game.engine");
    assert(deps[1] == "utils.mp");

    // Cycle detection should be deterministic and explicit.
    stdlib.call("writeFile", {Value(modBase + "/cycle_a.mp"), Value(std::string("import cycle_b\nprint 1\n"))});
    stdlib.call("writeFile", {Value(modBase + "/cycle_b.mp"), Value(std::string("import cycle_a\nprint 2\n"))});
    bool cycleRaised = false;
    try {
        moduleSystem.clearCache();
        (void)moduleSystem.loadImportedModule("cycle_a", modBase);
    } catch (const std::runtime_error& ex) {
        cycleRaised = std::string(ex.what()).find("cyclic import/use detected") != std::string::npos;
    }
    assert(cycleRaised);

    // Real runtime: function calling + scoping + runtime error types.
    const std::string runtimeSource = R"(
fn add(a, b) {
  return a + b
}
fn addBase(a, b = 5) {
  return a + b
}
fn sumAll(...nums) {
  return len(nums)
}
var x = add(5, 7)
var y = addBase(10)
var argc = sumAll(1, 2, 3, 4)
var flag = true and not false
if flag and x == 12 {
  x = x + 1
} else {
  x = 0
}
while x < 20 {
  x = x + 2
}
when x == 21 {
  set x = x + 1
}
loop 2 {
  set x = x + 1
}
repeat while x < 26 {
  set x = x + 1
}
switch x {
  case 26 {
    set x = x + 1
  }
  default {
    set x = 0
  }
}
match y {
  case 15 {
    set y = y + 1
  }
  default {
    set y = 0
  }
}
try {
  set missingVar = 1
} catch {
  set y = y + 1
}
var arr = [1, 2, 3]
var arrLen = len(arr)
var sum = 0
for (var i = 0; i < 3; i = i + 1) {
  sum = sum + i
}
)";
    magphos::lexer::Lexer runtimeLexer;
    magphos::parser::Parser runtimeParser;
    const auto runtimeParse = runtimeParser.parse(runtimeLexer.tokenize(runtimeSource));
    assert(runtimeParse.errors.empty());

    RuntimeEngine engine;
    engine.loadProgram(runtimeParse.program);
    assert(engine.globals()->get("x").asNumber() == 27.0);
    assert(engine.globals()->get("y").asNumber() == 17.0);
    assert(engine.globals()->get("argc").asNumber() == 4.0);
    assert(engine.globals()->get("flag").asBoolean());
    assert(engine.globals()->get("sum").asNumber() == 3.0);
    assert(engine.globals()->get("arrLen").asNumber() == 3.0);

    const std::string aritySource = R"(
fn add(a, b) {
  return a + b
}
var x = add(1)
)";
    const auto arityParse = runtimeParser.parse(runtimeLexer.tokenize(aritySource));
    bool arityRaised = false;
    try {
        RuntimeEngine badEngine;
        badEngine.loadProgram(arityParse.program);
    } catch (const RuntimeError& ex) {
        arityRaised = ex.code() == RuntimeErrorCode::ArityError;
    }
    assert(arityRaised);

    const std::string semanticBad = magphos::interpreter::analyzeProgram("print unknownVar\n");
    assert(semanticBad.find("Semantic error: Undefined identifier: unknownVar") != std::string::npos);
    const std::string duplicateDecl = magphos::interpreter::analyzeProgram("var x = 1\nvar x = 2\n");
    assert(duplicateDecl.find("Duplicate declaration in same scope") != std::string::npos);
    const std::string badReturn = magphos::interpreter::analyzeProgram("return 1\n");
    assert(badReturn.find("'return' is only allowed inside functions") != std::string::npos);
    const std::string badSet = magphos::interpreter::analyzeProgram("set y = 1\n");
    assert(badSet.find("'set' requires an existing variable") != std::string::npos);

    // Runtime error-code semantics should be stable.
    bool unknownFunctionRaised = false;
    try {
        RuntimeEngine unknownFnEngine;
        const auto parsed = runtimeParser.parse(runtimeLexer.tokenize("var x = unknownFn(1)\n"));
        unknownFnEngine.loadProgram(parsed.program);
    } catch (const RuntimeError& ex) {
        unknownFunctionRaised = ex.code() == RuntimeErrorCode::NameError;
        assert(magphos::runtime::runtimeErrorCodeName(ex.code()) == "RUNTIME_NAME_ERROR");
    }
    assert(unknownFunctionRaised);

    return 0;
}
