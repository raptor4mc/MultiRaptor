#include "runtime/engine/module_system.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "compiler/ast/nodes.h"
#include "compiler/lexer/lexer.h"
#include "compiler/parser/parser.h"

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
    return loadPathInternal(path);
}

std::string ModuleSystem::loadUsePath(const std::string& relativePath, const std::string& baseDir) const {
    const std::string path = resolveUsePath(relativePath, baseDir);
    return loadPathInternal(path);
}

std::string ModuleSystem::loadPathInternal(const std::string& path) const {
    const auto cached = cache_.find(path);
    if (cached != cache_.end()) {
        return cached->second;
    }
    if (loadStack_.find(path) != loadStack_.end()) {
        throw std::runtime_error("ModuleSystem: cyclic import/use detected for " + path);
    }

    loadStack_.insert(path);
    try {
        const std::string content = readFile(path);

        lexer::Lexer lexer;
        parser::Parser parser;
        const auto parseResult = parser.parse(lexer.tokenize(content));
        if (parseResult.errors.empty()) {
            const std::string childBaseDir = std::filesystem::path(path).parent_path().string();
            for (const auto& statement : parseResult.program.statements) {
                if (statement.kind == ast::StmtKind::Import) {
                    const std::string depPath = resolveModulePath(statement.name, childBaseDir);
                    (void)loadPathInternal(depPath);
                } else if (statement.kind == ast::StmtKind::Use) {
                    const std::string depPath = resolveUsePath(statement.name, childBaseDir);
                    (void)loadPathInternal(depPath);
                }
            }
        }

        cache_[path] = content;
        loadStack_.erase(path);
        return content;
    } catch (...) {
        loadStack_.erase(path);
        throw;
    }
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
