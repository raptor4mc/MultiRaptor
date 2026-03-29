#pragma once

#include <stdexcept>
#include <string>

namespace magphos::runtime {

enum class RuntimeErrorCode {
    NameError,
    TypeError,
    ArityError,
    ScopeError,
    RuntimeFailure,
};

class RuntimeError : public std::runtime_error {
  public:
    RuntimeError(RuntimeErrorCode code, std::string message, std::string hint = "");

    RuntimeErrorCode code() const;
    const std::string& hint() const;

  private:
    RuntimeErrorCode code_;
    std::string hint_;
};

std::string runtimeErrorCodeName(RuntimeErrorCode code);
std::string renderRuntimeError(const RuntimeError& error);

} // namespace magphos::runtime
