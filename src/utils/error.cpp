#include "utils/error.h"

namespace magphos::utils {

std::string formatError(const std::string& message) {
    return "MagPhos error: " + message;
}

} // namespace magphos::utils
