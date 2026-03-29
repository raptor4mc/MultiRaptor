#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "runtime/value.h"

namespace magphos::runtime {

using NativeFunction = std::function<Value(const std::vector<Value>&)>;

class StandardLibrary {
  public:
    StandardLibrary();

    bool has(const std::string& name) const;
    Value call(const std::string& name, const std::vector<Value>& args) const;
    std::vector<std::string> list() const;

  private:
    std::unordered_map<std::string, NativeFunction> functions_;

    void registerCore();
    void registerMath();
    void registerStrings();
    void registerArrays();
    void registerFileIO();
    void registerGameGraphics();
    void registerNetworking();
    void registerInteroperability();
};

} // namespace magphos::runtime
