#include <cassert>
#include <memory>
#include <string>

#include "interpreter/interpreter.h"
#include "runtime/environment.h"
#include "runtime/value.h"

int main() {
    const std::string ok = magphos::interpreter::analyzeProgram("print (1 + 2) * 3\n");
    assert(ok == "ok");

    const std::string bad = magphos::interpreter::analyzeProgram("fn broken(a {\n");
    assert(bad.rfind("errors=", 0) == 0);

    using magphos::runtime::TypeKind;
    using magphos::runtime::Value;

    const Value numberValue(42.0);
    assert(numberValue.type() == TypeKind::Number);
    assert(numberValue.asNumber() == 42.0);

    const Value stringValue(std::string("magphos"));
    assert(stringValue.type() == TypeKind::String);
    assert(stringValue.asString() == "magphos");

    const Value boolValue(true);
    assert(boolValue.type() == TypeKind::Boolean);
    assert(boolValue.asBoolean());

    const Value nullValue = Value::makeNull();
    assert(nullValue.type() == TypeKind::Null);
    assert(nullValue.isNull());

    const Value functionValue = Value::makeFunction("add", {"a", "b"});
    assert(functionValue.type() == TypeKind::Function);
    assert(functionValue.asFunction().params.size() == 2);

    magphos::runtime::ObjectValue fields;
    fields.fields["name"] = std::make_shared<Value>(Value(std::string("core")));
    const Value objectValue = Value::makeObject(fields);
    assert(objectValue.type() == TypeKind::Object);
    assert(objectValue.asObject().fields.find("name") != objectValue.asObject().fields.end());

    magphos::runtime::ArrayValue list;
    list.elements.push_back(std::make_shared<Value>(Value(1.0)));
    list.elements.push_back(std::make_shared<Value>(Value(2.0)));
    const Value arrayValue = Value::makeArray(list);
    assert(arrayValue.type() == TypeKind::Array);
    assert(arrayValue.asArray().elements.size() == 2);

    magphos::runtime::MapValue mapEntries;
    mapEntries.entries["k"] = std::make_shared<Value>(Value(10.0));
    const Value mapValue = Value::makeMap(mapEntries);
    assert(mapValue.type() == TypeKind::Map);
    assert(mapValue.asMap().entries.find("k") != mapValue.asMap().entries.end());

    const Value classValue = Value::makeClass("Widget", fields);
    assert(classValue.type() == TypeKind::Class);
    assert(classValue.asClass().name == "Widget");

    const Value structValue = Value::makeStruct("Point", fields);
    assert(structValue.type() == TypeKind::Struct);
    assert(structValue.asStruct().name == "Point");

    const Value enumValue = Value::makeEnum("Color", "Red");
    assert(enumValue.type() == TypeKind::Enum);
    assert(enumValue.asEnum().variant == "Red");

    magphos::runtime::Environment env;
    env.set("answer", numberValue);
    assert(env.get("answer").asNumber() == 42.0);
    assert(env.get("missing").isNull());

    return 0;
}
