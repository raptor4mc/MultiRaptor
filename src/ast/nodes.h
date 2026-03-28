#pragma once

#include <string>

namespace magphos::ast {

struct Node {
    std::string kind;
};

Node makeProgramNode();

} // namespace magphos::ast
