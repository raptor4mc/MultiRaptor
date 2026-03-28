#pragma once

#include <string>
#include <vector>

#include "parser/parser.h"

namespace magphos::runtime {

class ModuleSystem {
  public:
    std::string resolveModulePath(const std::string& moduleName, const std::string& baseDir) const;
    std::string resolveUsePath(const std::string& relativePath, const std::string& baseDir) const;

    std::string loadImportedModule(const std::string& moduleName, const std::string& baseDir) const;
    std::string loadUsePath(const std::string& relativePath, const std::string& baseDir) const;

    std::vector<std::string> collectDependencies(const parser::ParseResult& parseResult) const;
};

} // namespace magphos::runtime
