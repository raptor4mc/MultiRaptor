#include "parser/parser.h"

#include "utils/string_utils.h"

namespace magphos::parser {

std::string normalizeLine(const std::string& line) {
    return magphos::utils::trim(line);
}

} // namespace magphos::parser
