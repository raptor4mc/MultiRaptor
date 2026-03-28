#pragma once

#include <string>
#include <unordered_map>

#include "runtime/value.h"

namespace magphos::runtime {

class Environment {
  public:
    void set(const std::string& name, Value value);
    Value get(const std::string& name) const;

  private:
    std::unordered_map<std::string, Value> values_;
};

} // namespace magphos::runtime
