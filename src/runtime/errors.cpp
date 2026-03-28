#include "runtime/errors.h"

namespace magphos::runtime {

RuntimeError::RuntimeError(RuntimeErrorCode code, std::string message, std::string hint)
    : std::runtime_error(std::move(message)), code_(code), hint_(std::move(hint)) {}

RuntimeErrorCode RuntimeError::code() const {
    return code_;
}

const std::string& RuntimeError::hint() const {
    return hint_;
}

} // namespace magphos::runtime
