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

    for (const auto& root : moduleSearchRoots(baseDir)) {
        const std::filesystem::path candidate = std::filesystem::path(root) / (modulePath + ".mp");
        if (std::filesystem::exists(candidate)) {
            return candidate.lexically_normal().string();
        }
    }

    // Fall back to the caller base dir path for deterministic diagnostics.
    const std::filesystem::path fallback = std::filesystem::path(baseDir) / (modulePath + ".mp");
    return fallback.lexically_normal().string();
}

std::string ModuleSystem::resolveUsePath(const std::string& relativePath, const std::string& baseDir) const {
    std::filesystem::path full = std::filesystem::path(baseDir) / relativePath;
    return full.lexically_normal().string();
}

std::vector<std::string> ModuleSystem::moduleSearchRoots(const std::string& baseDir) const {
    std::vector<std::string> roots;

    const std::filesystem::path base = std::filesystem::path(baseDir).lexically_normal();
    roots.push_back(base.string());

    try {
        const std::filesystem::path cwd = std::filesystem::current_path();
        const std::filesystem::path cwdLibLower = cwd / "lib";
        const std::filesystem::path cwdLibUpper = cwd / "Lib";
        if (std::filesystem::exists(cwdLibLower)) {
            roots.push_back(cwdLibLower.lexically_normal().string());
        }
        if (std::filesystem::exists(cwdLibUpper)) {
            roots.push_back(cwdLibUpper.lexically_normal().string());
        }
    } catch (...) {
        // Ignore cwd probing failures; baseDir resolution still works.
    }

    // Include parent-chain stdlib folders so nested program directories still resolve lib imports.
    std::filesystem::path cursor = base;
    while (!cursor.empty()) {
        const std::filesystem::path lower = cursor / "lib";
        const std::filesystem::path upper = cursor / "Lib";
        if (std::filesystem::exists(lower)) {
            roots.push_back(lower.lexically_normal().string());
        }
        if (std::filesystem::exists(upper)) {
            roots.push_back(upper.lexically_normal().string());
        }
        if (!cursor.has_parent_path() || cursor.parent_path() == cursor) {
            break;
        }
        cursor = cursor.parent_path();
    }

    // Stable de-duplication while preserving search order.
    std::unordered_set<std::string> seen;
    std::vector<std::string> deduped;
    deduped.reserve(roots.size());
    for (const auto& root : roots) {
        if (seen.insert(root).second) {
            deduped.push_back(root);
        }
    }
    return deduped;
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
