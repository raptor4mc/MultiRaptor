#pragma once

#include <string>

namespace magphos::runtime {

class Value {
  public:
    Value() = default;
    explicit Value(std::string text);

    const std::string& asString() const;

  private:
    std::string text_;
};

} // namespace magphos::runtime
