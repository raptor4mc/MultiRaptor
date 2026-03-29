#pragma once

#include <string>
#include <vector>

#include "compiler/ast/nodes.h"

namespace magphos::semantic {

struct SemanticIssue {
    std::string message;
};

std::vector<SemanticIssue> analyze(const ast::Program& program);
std::string renderIssues(const std::vector<SemanticIssue>& issues);

} // namespace magphos::semantic
