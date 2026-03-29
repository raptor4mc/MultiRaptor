#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "interpreter/interpreter.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "runtime/engine.h"
#include "runtime/errors.h"
#include "runtime/module_system.h"
#include "semantic/analyzer.h"

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        return "";
    }
    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}

std::string jsonEscape(const std::string& input) {
    std::ostringstream out;
    for (const char c : input) {
        switch (c) {
            case '\\':
                out << "\\\\";
                break;
            case '"':
                out << "\\\"";
                break;
            case '\n':
                out << "\\n";
                break;
            case '\r':
                out << "\\r";
                break;
            case '\t':
                out << "\\t";
                break;
            default:
                out << c;
                break;
        }
    }
    return out.str();
}

std::string tokenTypeName(magphos::lexer::TokenType type) {
    using magphos::lexer::TokenType;
    switch (type) {
        case TokenType::Identifier: return "Identifier";
        case TokenType::Number: return "Number";
        case TokenType::String: return "String";
        case TokenType::LeftParen: return "LeftParen";
        case TokenType::RightParen: return "RightParen";
        case TokenType::LeftBrace: return "LeftBrace";
        case TokenType::RightBrace: return "RightBrace";
        case TokenType::LeftBracket: return "LeftBracket";
        case TokenType::RightBracket: return "RightBracket";
        case TokenType::Comma: return "Comma";
        case TokenType::Dot: return "Dot";
        case TokenType::Semicolon: return "Semicolon";
        case TokenType::Plus: return "Plus";
        case TokenType::Minus: return "Minus";
        case TokenType::Star: return "Star";
        case TokenType::Slash: return "Slash";
        case TokenType::Bang: return "Bang";
        case TokenType::BangEqual: return "BangEqual";
        case TokenType::Equal: return "Equal";
        case TokenType::EqualEqual: return "EqualEqual";
        case TokenType::Less: return "Less";
        case TokenType::LessEqual: return "LessEqual";
        case TokenType::Greater: return "Greater";
        case TokenType::GreaterEqual: return "GreaterEqual";
        case TokenType::Newline: return "Newline";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Invalid: return "Invalid";
    }
    return "Unknown";
}

enum class ErrorFormat {
    Short,
    Long,
    Json
};

struct CliResponse {
    bool ok = false;
    std::string command;
    int code = 0;
    std::string message;
    std::vector<std::string> errors;
    std::vector<std::string> deps;
    std::vector<magphos::lexer::Token> tokens;
    std::vector<std::pair<std::string, std::string>> moduleEdges;
    std::string stdoutText;
};

std::string shortenError(const std::string& text) {
    const std::size_t newline = text.find('\n');
    if (newline == std::string::npos) return text;
    return text.substr(0, newline);
}

void emitResponse(const CliResponse& response, bool jsonMode, ErrorFormat errorFormat) {
    if (jsonMode) {
        std::cout << "{";
        std::cout << "\"ok\":" << (response.ok ? "true" : "false");
        std::cout << ",\"command\":\"" << jsonEscape(response.command) << "\"";
        std::cout << ",\"code\":" << response.code;
        std::cout << ",\"message\":\"" << jsonEscape(response.message) << "\"";
        std::cout << ",\"errors\":[";
        for (std::size_t i = 0; i < response.errors.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << "\"" << jsonEscape(response.errors[i]) << "\"";
        }
        std::cout << "]";
        std::cout << ",\"deps\":[";
        for (std::size_t i = 0; i < response.deps.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << "\"" << jsonEscape(response.deps[i]) << "\"";
        }
        std::cout << "]";
        std::cout << ",\"tokens\":[";
        for (std::size_t i = 0; i < response.tokens.size(); ++i) {
            if (i) std::cout << ",";
            const auto& token = response.tokens[i];
            std::cout << "{"
                      << "\"type\":\"" << tokenTypeName(token.type) << "\","
                      << "\"lexeme\":\"" << jsonEscape(token.lexeme) << "\","
                      << "\"line\":" << token.line << ","
                      << "\"column\":" << token.column
                      << "}";
        }
        std::cout << "]";
        std::cout << ",\"moduleGraph\":[";
        for (std::size_t i = 0; i < response.moduleEdges.size(); ++i) {
            if (i) std::cout << ",";
            std::cout << "{"
                      << "\"from\":\"" << jsonEscape(response.moduleEdges[i].first) << "\","
                      << "\"to\":\"" << jsonEscape(response.moduleEdges[i].second) << "\","
                      << "\"type\":\"import\""
                      << "}";
        }
        std::cout << "]";
        std::cout << ",\"stdout\":\"" << jsonEscape(response.stdoutText) << "\"";
        std::cout << "}\n";
        return;
    }

    if (response.ok) {
        if (!response.stdoutText.empty()) std::cout << response.stdoutText;
        if (!response.message.empty()) std::cout << response.message << "\n";
        if (!response.deps.empty()) {
            for (const auto& dep : response.deps) std::cout << dep << "\n";
        }
        if (!response.tokens.empty()) {
            for (const auto& token : response.tokens) {
                std::cout << token.line << ":" << token.column << " '" << token.lexeme << "'\n";
            }
        }
        if (!response.moduleEdges.empty()) {
            for (const auto& edge : response.moduleEdges) {
                std::cout << edge.first << " -> " << edge.second << "\n";
            }
        }
        return;
    }

    if (response.errors.empty()) {
        std::cerr << response.message << "\n";
        return;
    }

    if (errorFormat == ErrorFormat::Short) {
        std::cerr << shortenError(response.errors.front()) << "\n";
        return;
    }
    for (const auto& err : response.errors) {
        std::cerr << err << "\n";
    }
}
}

int main(int argc, char** argv) {
    std::vector<std::string> positional;
    bool jsonMode = false;
    ErrorFormat errorFormat = ErrorFormat::Long;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--json") {
            jsonMode = true;
            continue;
        }
        if (arg == "--format-errors") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for --format-errors\n";
                return 2;
            }
            const std::string value = argv[++i];
            if (value == "short") errorFormat = ErrorFormat::Short;
            else if (value == "long") errorFormat = ErrorFormat::Long;
            else if (value == "json") errorFormat = ErrorFormat::Json;
            else {
                std::cerr << "Unknown --format-errors value: " << value << "\n";
                return 2;
            }
            continue;
        }
        positional.push_back(arg);
    }

    if (errorFormat == ErrorFormat::Json) jsonMode = true;

    if (positional.empty()) {
        std::cout << "MagPhos CLI\n";
        std::cout << "Usage:\n";
        std::cout << "  magphos_cli --version\n";
        std::cout << "  magphos_cli --about\n";
        std::cout << "  magphos_cli --check <program.mp>\n";
        std::cout << "  magphos_cli --run <program.mp>\n";
        std::cout << "  magphos_cli --deps <program.mp> [baseDir]\n";
        std::cout << "  magphos_cli --module-graph <program.mp> [baseDir]\n";
        std::cout << "  magphos_cli --tokens <program.mp>\n";
        return 0;
    }

    const std::string cmd = positional[0];
    if (cmd == "--version") {
        std::cout << "MagPhos CLI v0.1.0\n";
        return 0;
    }
    if (cmd == "--about") {
        std::cout << "MagPhos includes a native lexer/parser/runtime toolchain.\n";
        return 0;
    }
    if (cmd == "--check" && positional.size() >= 2) {
        CliResponse response;
        response.command = "--check";
        const std::string source = readFile(positional[1]);
        if (source.empty()) {
            response.code = 2;
            response.message = "Cannot read file: " + positional[1];
            response.errors.push_back(response.message);
            emitResponse(response, jsonMode, errorFormat);
            return 2;
        }
        const std::string result = magphos::interpreter::analyzeProgram(source);
        if (result == "ok") {
            response.ok = true;
            response.message = "ok";
            emitResponse(response, jsonMode, errorFormat);
            return 0;
        }
        response.code = 3;
        response.message = "analysis failed";
        response.errors.push_back(result);
        emitResponse(response, jsonMode, errorFormat);
        return 3;
    }
    if (cmd == "--run" && positional.size() >= 2) {
        CliResponse response;
        response.command = "--run";
        const std::string source = readFile(positional[1]);
        if (source.empty()) {
            response.code = 2;
            response.message = "Cannot read file: " + positional[1];
            response.errors.push_back(response.message);
            emitResponse(response, jsonMode, errorFormat);
            return 2;
        }
        magphos::lexer::Lexer lexer;
        magphos::parser::Parser parser;
        const auto parsed = parser.parse(lexer.tokenize(source));
        if (!parsed.errors.empty()) {
            response.code = 3;
            response.message = "parse failed";
            response.errors.push_back(magphos::parser::renderErrors(parsed.errors, source));
            emitResponse(response, jsonMode, errorFormat);
            return 3;
        }
        const auto semanticIssues = magphos::semantic::analyze(parsed.program);
        if (!semanticIssues.empty()) {
            response.code = 3;
            response.message = "semantic analysis failed";
            response.errors.push_back(magphos::semantic::renderIssues(semanticIssues));
            emitResponse(response, jsonMode, errorFormat);
            return 3;
        }
        std::ostringstream capture;
        auto* oldOut = std::cout.rdbuf(capture.rdbuf());
        try {
            magphos::runtime::RuntimeEngine engine;
            engine.loadProgram(parsed.program);
        } catch (const std::exception& ex) {
            std::cout.rdbuf(oldOut);
            response.code = 3;
            response.message = "runtime failed";
            response.errors.push_back(ex.what());
            emitResponse(response, jsonMode, errorFormat);
            return 3;
        }
        std::cout.rdbuf(oldOut);
        response.ok = true;
        response.message = "ok";
        response.stdoutText = capture.str();
        emitResponse(response, jsonMode, errorFormat);
        return 0;
    }
    if (cmd == "--tokens" && positional.size() >= 2) {
        CliResponse response;
        response.command = "--tokens";
        const std::string source = readFile(positional[1]);
        if (source.empty()) {
            response.code = 2;
            response.message = "Cannot read file: " + positional[1];
            response.errors.push_back(response.message);
            emitResponse(response, jsonMode, errorFormat);
            return 2;
        }
        magphos::lexer::Lexer lexer;
        response.ok = true;
        response.command = "--tokens";
        response.tokens = lexer.tokenize(source);
        response.message = "ok";
        emitResponse(response, jsonMode, errorFormat);
        return 0;
    }
    if ((cmd == "--deps" || cmd == "--module-graph") && positional.size() >= 2) {
        CliResponse response;
        response.command = cmd;
        const std::string source = readFile(positional[1]);
        if (source.empty()) {
            response.code = 2;
            response.message = "Cannot read file: " + positional[1];
            response.errors.push_back(response.message);
            emitResponse(response, jsonMode, errorFormat);
            return 2;
        }
        magphos::lexer::Lexer lexer;
        magphos::parser::Parser parser;
        const auto parsed = parser.parse(lexer.tokenize(source));
        if (!parsed.errors.empty()) {
            response.code = 3;
            response.message = "parse failed";
            response.errors.push_back(magphos::parser::renderErrors(parsed.errors, source));
            emitResponse(response, jsonMode, errorFormat);
            return 3;
        }
        magphos::runtime::ModuleSystem modules;
        const auto deps = modules.collectDependencies(parsed);
        response.ok = true;
        response.message = "ok";
        if (cmd == "--module-graph") {
            for (const auto& dep : deps) {
                response.moduleEdges.push_back({positional[1], dep});
            }
        } else {
            response.deps = deps;
        }
        emitResponse(response, jsonMode, errorFormat);
        return 0;
    }

    CliResponse response;
    response.command = cmd;
    response.code = 2;
    response.message = "Unknown command: " + cmd;
    response.errors.push_back(response.message);
    emitResponse(response, jsonMode, errorFormat);
    return 2;
}
