#include "runtime/engine/module_system.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "compiler/ast/nodes.h"

namespace magphos::runtime {

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("ModuleSystem: cannot open file " + path);
    }

    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}

} // namespace

std::string ModuleSystem::resolveModulePath(const std::string& moduleName, const std::string& baseDir) const {
    std::string modulePath = moduleName;
    for (char& c : modulePath) {
        if (c == '.') {
            c = '/';
        }
    }

    std::filesystem::path full = std::filesystem::path(baseDir) / (modulePath + ".mp");
    return full.lexically_normal().string();
}

std::string ModuleSystem::resolveUsePath(const std::string& relativePath, const std::string& baseDir) const {
    std::filesystem::path full = std::filesystem::path(baseDir) / relativePath;
    return full.lexically_normal().string();
}

std::string ModuleSystem::loadImportedModule(const std::string& moduleName, const std::string& baseDir) const {
    const std::string path = resolveModulePath(moduleName, baseDir);
    const auto cached = cache_.find(path);
    if (cached != cache_.end()) {
        return cached->second;
    }
    if (loadStack_.find(path) != loadStack_.end()) {
        throw std::runtime_error("ModuleSystem: cyclic import detected for " + path);
    }
    loadStack_.insert(path);
    const std::string content = readFile(path);
    cache_[path] = content;
    loadStack_.erase(path);
    return content;
}

std::string ModuleSystem::loadUsePath(const std::string& relativePath, const std::string& baseDir) const {
    const std::string path = resolveUsePath(relativePath, baseDir);
    const auto cached = cache_.find(path);
    if (cached != cache_.end()) {
        return cached->second;
    }
    if (loadStack_.find(path) != loadStack_.end()) {
        throw std::runtime_error("ModuleSystem: cyclic use-path detected for " + path);
    }
    loadStack_.insert(path);
    const std::string content = readFile(path);
    cache_[path] = content;
    loadStack_.erase(path);
    return content;
}

std::vector<std::string> ModuleSystem::collectDependencies(const parser::ParseResult& parseResult) const {
    std::vector<std::string> dependencies;
    for (const auto& statement : parseResult.program.statements) {
        if (statement.kind == ast::StmtKind::Import || statement.kind == ast::StmtKind::Use) {
            dependencies.push_back(statement.name);
        }
    }
    return dependencies;
}

void ModuleSystem::clearCache() {
    cache_.clear();
    loadStack_.clear();
}

} // namespace magphos::runtime
