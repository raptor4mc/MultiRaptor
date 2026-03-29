#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "runtime/engine/errors.h"
#include "runtime/engine/value.h"

namespace magphos::runtime {

class Environment {
  public:
    Environment();
    explicit Environment(std::shared_ptr<Environment> parent);

    void define(const std::string& name, Value value);
    void assign(const std::string& name, Value value);
    Value get(const std::string& name) const;

    std::shared_ptr<Environment> parent() const;

    // Compatibility methods.
    void set(const std::string& name, Value value);

  private:
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment> parent_;
};

} // namespace magphos::runtime
