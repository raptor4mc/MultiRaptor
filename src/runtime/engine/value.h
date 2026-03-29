#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

namespace magphos::runtime {

class Value;
using ValuePtr = std::shared_ptr<Value>;

struct FunctionValue {
    std::string name;
    std::vector<std::string> params;
};

struct ObjectValue {
    std::unordered_map<std::string, ValuePtr> fields;
};

struct ArrayValue {
    std::vector<ValuePtr> elements;
};

struct MapValue {
    std::unordered_map<std::string, ValuePtr> entries;
};

struct ClassValue {
    std::string name;
    ObjectValue fields;
};

struct StructValue {
    std::string name;
    ObjectValue fields;
};

struct EnumValue {
    std::string name;
    std::string variant;
};

enum class TypeKind {
    Null,
    Number,
    String,
    Boolean,
    Function,
    Object,
    Array,
    Map,
    Class,
    Struct,
    Enum,
};

class Value {
  public:
    using Payload = std::variant<std::monostate,
                                 double,
                                 std::string,
                                 bool,
                                 FunctionValue,
                                 ObjectValue,
                                 ArrayValue,
                                 MapValue,
                                 ClassValue,
                                 StructValue,
                                 EnumValue>;

    Value();
    explicit Value(double number);
    explicit Value(std::string text);
    explicit Value(bool boolean);

    static Value makeNull();
    static Value makeFunction(std::string name, std::vector<std::string> params);
    static Value makeObject(ObjectValue fields = {});
    static Value makeArray(ArrayValue values = {});
    static Value makeMap(MapValue entries = {});
    static Value makeClass(std::string name, ObjectValue fields = {});
    static Value makeStruct(std::string name, ObjectValue fields = {});
    static Value makeEnum(std::string name, std::string variant);

    TypeKind type() const;

    bool isNull() const;
    double asNumber() const;
    const std::string& asString() const;
    bool asBoolean() const;
    const FunctionValue& asFunction() const;
    const ObjectValue& asObject() const;
    const ArrayValue& asArray() const;
    const MapValue& asMap() const;
    const ClassValue& asClass() const;
    const StructValue& asStruct() const;
    const EnumValue& asEnum() const;

  private:
    explicit Value(Payload payload, TypeKind kind);

    Payload payload_;
    TypeKind kind_;
};

} // namespace magphos::runtime
