#include "runtime/enviroment.h"

namespace magphos::runtime {

void Enviroment::set(const std::string& name, const std::string& value) {
    values_[name] = value;
}

std::string Enviroment::get(const std::string& name) const {
    const auto it = values_.find(name);
    if (it == values_.end()) {
        return "";
    }
    return it->second;
}

} // namespace magphos::runtime
