#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "compiler/parser/parser.h"

namespace magphos::runtime {

class ModuleSystem {
  public:
    std::string resolveModulePath(const std::string& moduleName, const std::string& baseDir) const;
    std::string resolveUsePath(const std::string& relativePath, const std::string& baseDir) const;

    std::string loadImportedModule(const std::string& moduleName, const std::string& baseDir) const;
    std::string loadUsePath(const std::string& relativePath, const std::string& baseDir) const;

    std::vector<std::string> collectDependencies(const parser::ParseResult& parseResult) const;

    void clearCache();

  private:
    mutable std::unordered_map<std::string, std::string> cache_;
    mutable std::unordered_set<std::string> loadStack_;
};

} // namespace magphos::runtime
