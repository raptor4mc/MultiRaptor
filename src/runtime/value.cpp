#include "runtime/value.h"

#include <stdexcept>

namespace magphos::runtime {

Value::Value() : payload_(std::monostate{}), kind_(TypeKind::Null) {}

Value::Value(double number) : payload_(number), kind_(TypeKind::Number) {}

Value::Value(std::string text) : payload_(std::move(text)), kind_(TypeKind::String) {}

Value::Value(bool boolean) : payload_(boolean), kind_(TypeKind::Boolean) {}

Value::Value(Payload payload, TypeKind kind) : payload_(std::move(payload)), kind_(kind) {}

Value Value::makeNull() { return Value(); }

Value Value::makeFunction(std::string name, std::vector<std::string> params) {
    return Value(Payload(FunctionValue{std::move(name), std::move(params)}), TypeKind::Function);
}

Value Value::makeObject(ObjectValue fields) {
    return Value(Payload(std::move(fields)), TypeKind::Object);
}

Value Value::makeArray(ArrayValue values) {
    return Value(Payload(std::move(values)), TypeKind::Array);
}

Value Value::makeMap(MapValue entries) {
    return Value(Payload(std::move(entries)), TypeKind::Map);
}

Value Value::makeClass(std::string name, ObjectValue fields) {
    return Value(Payload(ClassValue{std::move(name), std::move(fields)}), TypeKind::Class);
}

Value Value::makeStruct(std::string name, ObjectValue fields) {
    return Value(Payload(StructValue{std::move(name), std::move(fields)}), TypeKind::Struct);
}

Value Value::makeEnum(std::string name, std::string variant) {
    return Value(Payload(EnumValue{std::move(name), std::move(variant)}), TypeKind::Enum);
}

TypeKind Value::type() const { return kind_; }

bool Value::isNull() const { return kind_ == TypeKind::Null; }

double Value::asNumber() const {
    if (kind_ != TypeKind::Number) {
        throw std::runtime_error("Value is not a number.");
    }
    return std::get<double>(payload_);
}

const std::string& Value::asString() const {
    if (kind_ != TypeKind::String) {
        throw std::runtime_error("Value is not a string.");
    }
    return std::get<std::string>(payload_);
}

bool Value::asBoolean() const {
    if (kind_ != TypeKind::Boolean) {
        throw std::runtime_error("Value is not a boolean.");
    }
    return std::get<bool>(payload_);
}

const FunctionValue& Value::asFunction() const {
    if (kind_ != TypeKind::Function) {
        throw std::runtime_error("Value is not a function.");
    }
    return std::get<FunctionValue>(payload_);
}

const ObjectValue& Value::asObject() const {
    if (kind_ != TypeKind::Object) {
        throw std::runtime_error("Value is not an object.");
    }
    return std::get<ObjectValue>(payload_);
}

const ArrayValue& Value::asArray() const {
    if (kind_ != TypeKind::Array) {
        throw std::runtime_error("Value is not an array.");
    }
    return std::get<ArrayValue>(payload_);
}

const MapValue& Value::asMap() const {
    if (kind_ != TypeKind::Map) {
        throw std::runtime_error("Value is not a map.");
    }
    return std::get<MapValue>(payload_);
}

const ClassValue& Value::asClass() const {
    if (kind_ != TypeKind::Class) {
        throw std::runtime_error("Value is not a class.");
    }
    return std::get<ClassValue>(payload_);
}

const StructValue& Value::asStruct() const {
    if (kind_ != TypeKind::Struct) {
        throw std::runtime_error("Value is not a struct.");
    }
    return std::get<StructValue>(payload_);
}

const EnumValue& Value::asEnum() const {
    if (kind_ != TypeKind::Enum) {
        throw std::runtime_error("Value is not an enum.");
    }
    return std::get<EnumValue>(payload_);
}

} // namespace magphos::runtime
