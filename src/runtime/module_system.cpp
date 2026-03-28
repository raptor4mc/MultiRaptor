#include "runtime/module_system.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "ast/nodes.h"

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
    return readFile(resolveModulePath(moduleName, baseDir));
}

std::string ModuleSystem::loadUsePath(const std::string& relativePath, const std::string& baseDir) const {
    return readFile(resolveUsePath(relativePath, baseDir));
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

} // namespace magphos::runtime
