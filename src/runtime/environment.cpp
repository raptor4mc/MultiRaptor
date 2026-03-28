#include "runtime/environment.h"

namespace magphos::runtime {

void Environment::set(const std::string& name, Value value) {
    values_[name] = std::move(value);
}

Value Environment::get(const std::string& name) const {
    const auto it = values_.find(name);
    if (it == values_.end()) {
        return Value::makeNull();
    }
    return it->second;
}

} // namespace magphos::runtime
