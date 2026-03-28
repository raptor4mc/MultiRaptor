#include "runtime/environment.h"

namespace magphos::runtime {

void Environment::set(const std::string& name, const std::string& value) { values_[name] = value; }

std::string Environment::get(const std::string& name) const {
    const auto it = values_.find(name);
    return it == values_.end() ? "" : it->second;
}

} // namespace magphos::runtime
