#include "utils/file_utils.h"

#include <fstream>
#include <sstream>

namespace magphos::utils {

std::string readTextFile(const std::string& path) {
    std::ifstream file(path);
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}

} // namespace magphos::utils
