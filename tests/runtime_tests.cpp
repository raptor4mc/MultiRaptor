#include <cassert>
#include <cmath>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "runtime/environment.h"
#include "runtime/module_system.h"
#include "runtime/stdlib.h"
#include "runtime/value.h"

int main() {
    const std::string ok = magphos::interpreter::analyzeProgram("print (1 + 2) * 3\n");
    assert(ok == "ok");

    const std::string bad = magphos::interpreter::analyzeProgram("fn broken(a {\n");
    assert(bad.rfind("errors=", 0) == 0);

    using magphos::runtime::ArrayValue;
    using magphos::runtime::ObjectValue;
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
    assert(env.get("missing").isNull());

    StandardLibrary stdlib;

    // Core built-ins.
    assert(stdlib.call("len", {stringValue}).asNumber() == 7.0);
    assert(stdlib.call("type", {arrayValue}).asString() == "array");
    assert(stdlib.call("toString", {enumValue}).asString() == "Color.Red");
    assert(stdlib.call("random", {}).type() == TypeKind::Number);
    assert(stdlib.call("time", {}).type() == TypeKind::Number);

    // Math built-ins.
    assert(std::abs(stdlib.call("sin", {Value(0.0)}).asNumber()) < 1e-9);
    assert(std::abs(stdlib.call("cos", {Value(0.0)}).asNumber() - 1.0) < 1e-9);
    assert(stdlib.call("sqrt", {Value(9.0)}).asNumber() == 3.0);
    assert(stdlib.call("abs", {Value(-5.0)}).asNumber() == 5.0);

    // String built-ins.
    const auto splitResult = stdlib.call("split", {Value(std::string("a,b,c")), Value(std::string(","))});
    assert(splitResult.type() == TypeKind::Array);
    assert(splitResult.asArray().elements.size() == 3);
    assert(stdlib.call("replace", {Value(std::string("aa")), Value(std::string("a")), Value(std::string("b"))}).asString() == "bb");
    assert(stdlib.call("substring", {Value(std::string("magphos")), Value(3.0), Value(3.0)}).asString() == "pho");

    // Array built-ins.
    const auto pushed = stdlib.call("push", {arrayValue, Value(3.0)});
    assert(pushed.asArray().elements.size() == 3);
    const auto popped = stdlib.call("pop", {pushed});
    assert(popped.asArray().elements.size() == 2);
    const auto mapped = stdlib.call("map", {arrayValue, Value(std::string("abs"))});
    assert(mapped.asArray().elements.size() == 2);
    const auto filtered = stdlib.call("filter", {pushed, Value(std::string("gt")), Value(1.5)});
    assert(filtered.asArray().elements.size() == 2);

    // File I/O built-ins.
    const std::string path = "/tmp/magphos_stdlib_test.txt";
    stdlib.call("writeFile", {Value(path), Value(std::string("hello"))});
    assert(stdlib.call("readFile", {Value(path)}).asString() == "hello");

    // Module/import system.
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

    const std::string depSource = "import game.engine\nuse \"utils.mp\"\nprint 1\n";
    magphos::lexer::Lexer depLexer;
    magphos::parser::Parser depParser;
    const auto depResult = depParser.parse(depLexer.tokenize(depSource));
    const auto deps = moduleSystem.collectDependencies(depResult);
    assert(deps.size() == 2);
    assert(deps[0] == "game.engine");
    assert(deps[1] == "utils.mp");

    return 0;
}
