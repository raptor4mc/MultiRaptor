#pragma once

#include <string>
#include <unordered_map>

namespace magphos::runtime {

class Environment {
  public:
    void set(const std::string& name, const std::string& value);
    std::string get(const std::string& name) const;

  private:
    std::unordered_map<std::string, std::string> values_;
};

} // namespace magphos::runtime
