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

std::string runtimeErrorCodeName(RuntimeErrorCode code) {
    switch (code) {
        case RuntimeErrorCode::NameError:
            return "RUNTIME_NAME_ERROR";
        case RuntimeErrorCode::TypeError:
            return "RUNTIME_TYPE_ERROR";
        case RuntimeErrorCode::ArityError:
            return "RUNTIME_ARITY_ERROR";
        case RuntimeErrorCode::ScopeError:
            return "RUNTIME_SCOPE_ERROR";
        case RuntimeErrorCode::RuntimeFailure:
            return "RUNTIME_FAILURE";
    }
    return "RUNTIME_UNKNOWN_ERROR";
}

std::string renderRuntimeError(const RuntimeError& error) {
    std::string text = runtimeErrorCodeName(error.code()) + ": " + error.what();
    if (!error.hint().empty()) {
        text += " | Hint: " + error.hint();
    }
    return text;
}

} // namespace magphos::runtime
