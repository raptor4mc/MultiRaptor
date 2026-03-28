#include "runtime/value.h"

namespace magphos::runtime {

Value::Value(std::string text) : text_(std::move(text)) {}

const std::string& Value::asString() const {
    return text_;
}

} // namespace magphos::runtime
