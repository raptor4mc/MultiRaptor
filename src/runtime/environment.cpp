#include "runtime/environment.h"

#include <utility>

namespace magphos::runtime {

Environment::Environment() = default;

Environment::Environment(std::shared_ptr<Environment> parent) : parent_(std::move(parent)) {}

void Environment::define(const std::string& name, Value value) {
    values_[name] = std::move(value);
}

void Environment::assign(const std::string& name, Value value) {
    const auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = std::move(value);
        return;
    }

    if (parent_) {
        parent_->assign(name, std::move(value));
        return;
    }

    throw RuntimeError(RuntimeErrorCode::NameError,
                       "Undefined variable: " + name,
                       "Declare the variable before assigning to it.");
}

Value Environment::get(const std::string& name) const {
    const auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }

    if (parent_) {
        return parent_->get(name);
    }

    throw RuntimeError(RuntimeErrorCode::NameError,
                       "Undefined variable: " + name,
                       "Check the variable name or define it in scope first.");
}

std::shared_ptr<Environment> Environment::parent() const {
    return parent_;
}

void Environment::set(const std::string& name, Value value) {
    define(name, std::move(value));
}

} // namespace magphos::runtime
