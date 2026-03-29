#include "runtime/stdlib.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>
#include <stdexcept>

namespace magphos::runtime {

namespace {

double requireNumber(const std::vector<Value>& args, std::size_t index, const std::string& fn) {
    if (index >= args.size()) {
        throw std::runtime_error(fn + ": missing number argument");
    }
    return args[index].asNumber();
}

std::string requireString(const std::vector<Value>& args, std::size_t index, const std::string& fn) {
    if (index >= args.size()) {
        throw std::runtime_error(fn + ": missing string argument");
    }
    return args[index].asString();
}

ArrayValue requireArray(const std::vector<Value>& args, std::size_t index, const std::string& fn) {
    if (index >= args.size()) {
        throw std::runtime_error(fn + ": missing array argument");
    }
    return args[index].asArray();
}

std::string typeName(const Value& value) {
    switch (value.type()) {
        case TypeKind::Null:
            return "null";
        case TypeKind::Number:
            return "number";
        case TypeKind::String:
            return "string";
        case TypeKind::Boolean:
            return "boolean";
        case TypeKind::Function:
            return "function";
        case TypeKind::Object:
            return "object";
        case TypeKind::Array:
            return "array";
        case TypeKind::Map:
            return "map";
        case TypeKind::Class:
            return "class";
        case TypeKind::Struct:
            return "struct";
        case TypeKind::Enum:
            return "enum";
    }
    return "unknown";
}

std::string valueToString(const Value& value) {
    switch (value.type()) {
        case TypeKind::Null:
            return "null";
        case TypeKind::Number: {
            std::ostringstream out;
            out << value.asNumber();
            return out.str();
        }
        case TypeKind::String:
            return value.asString();
        case TypeKind::Boolean:
            return value.asBoolean() ? "true" : "false";
        case TypeKind::Function:
            return "<function " + value.asFunction().name + ">";
        case TypeKind::Object:
            return "<object>";
        case TypeKind::Array:
            return "<array size=" + std::to_string(value.asArray().elements.size()) + ">";
        case TypeKind::Map:
            return "<map size=" + std::to_string(value.asMap().entries.size()) + ">";
        case TypeKind::Class:
            return "<class " + value.asClass().name + ">";
        case TypeKind::Struct:
            return "<struct " + value.asStruct().name + ">";
        case TypeKind::Enum:
            return value.asEnum().name + "." + value.asEnum().variant;
    }
    return "unknown";
}

} // namespace

StandardLibrary::StandardLibrary() {
    registerCore();
    registerMath();
    registerStrings();
    registerArrays();
    registerFileIO();
    registerGameGraphics();
    registerNetworking();
    registerInteroperability();
}

bool StandardLibrary::has(const std::string& name) const {
    return functions_.find(name) != functions_.end();
}

Value StandardLibrary::call(const std::string& name, const std::vector<Value>& args) const {
    const auto it = functions_.find(name);
    if (it == functions_.end()) {
        throw std::runtime_error("Unknown builtin: " + name);
    }
    return it->second(args);
}

std::vector<std::string> StandardLibrary::list() const {
    std::vector<std::string> names;
    names.reserve(functions_.size());
    for (const auto& [name, _] : functions_) {
        names.push_back(name);
    }
    std::sort(names.begin(), names.end());
    return names;
}

void StandardLibrary::registerCore() {
    functions_["len"] = [](const std::vector<Value>& args) {
        if (args.empty()) {
            throw std::runtime_error("len: missing argument");
        }
        const Value& v = args[0];
        switch (v.type()) {
            case TypeKind::String:
                return Value(static_cast<double>(v.asString().size()));
            case TypeKind::Array:
                return Value(static_cast<double>(v.asArray().elements.size()));
            case TypeKind::Object:
                return Value(static_cast<double>(v.asObject().fields.size()));
            case TypeKind::Map:
                return Value(static_cast<double>(v.asMap().entries.size()));
            default:
                return Value(0.0);
        }
    };

    functions_["type"] = [](const std::vector<Value>& args) {
        if (args.empty()) {
            throw std::runtime_error("type: missing argument");
        }
        return Value(typeName(args[0]));
    };

    functions_["toString"] = [](const std::vector<Value>& args) {
        if (args.empty()) {
            throw std::runtime_error("toString: missing argument");
        }
        return Value(valueToString(args[0]));
    };

    functions_["random"] = [](const std::vector<Value>&) {
        static thread_local std::mt19937 engine(std::random_device{}());
        static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
        return Value(dist(engine));
    };

    functions_["time"] = [](const std::vector<Value>&) {
        const auto now = std::chrono::system_clock::now();
        const auto epoch = now.time_since_epoch();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
        return Value(static_cast<double>(ms) / 1000.0);
    };
}

void StandardLibrary::registerMath() {
    functions_["sin"] = [](const std::vector<Value>& args) { return Value(std::sin(requireNumber(args, 0, "sin"))); };
    functions_["cos"] = [](const std::vector<Value>& args) { return Value(std::cos(requireNumber(args, 0, "cos"))); };
    functions_["sqrt"] = [](const std::vector<Value>& args) { return Value(std::sqrt(requireNumber(args, 0, "sqrt"))); };
    functions_["abs"] = [](const std::vector<Value>& args) { return Value(std::abs(requireNumber(args, 0, "abs"))); };
}

void StandardLibrary::registerStrings() {
    functions_["split"] = [](const std::vector<Value>& args) {
        const std::string input = requireString(args, 0, "split");
        const std::string delimiter = requireString(args, 1, "split");

        ArrayValue out;
        if (delimiter.empty()) {
            for (char c : input) {
                out.elements.push_back(std::make_shared<Value>(Value(std::string(1, c))));
            }
            return Value::makeArray(out);
        }

        std::size_t start = 0;
        while (start <= input.size()) {
            const std::size_t found = input.find(delimiter, start);
            if (found == std::string::npos) {
                out.elements.push_back(std::make_shared<Value>(Value(input.substr(start))));
                break;
            }
            out.elements.push_back(std::make_shared<Value>(Value(input.substr(start, found - start))));
            start = found + delimiter.size();
        }
        return Value::makeArray(out);
    };

    functions_["replace"] = [](const std::vector<Value>& args) {
        std::string input = requireString(args, 0, "replace");
        const std::string from = requireString(args, 1, "replace");
        const std::string to = requireString(args, 2, "replace");

        if (from.empty()) {
            return Value(input);
        }

        std::size_t pos = 0;
        while ((pos = input.find(from, pos)) != std::string::npos) {
            input.replace(pos, from.size(), to);
            pos += to.size();
        }
        return Value(input);
    };

    functions_["substring"] = [](const std::vector<Value>& args) {
        const std::string input = requireString(args, 0, "substring");
        const int start = static_cast<int>(requireNumber(args, 1, "substring"));
        int length = static_cast<int>(input.size()) - start;
        if (args.size() >= 3) {
            length = static_cast<int>(requireNumber(args, 2, "substring"));
        }

        const int boundedStart = std::max(0, std::min(start, static_cast<int>(input.size())));
        const int boundedLength = std::max(0, std::min(length, static_cast<int>(input.size()) - boundedStart));
        return Value(input.substr(static_cast<std::size_t>(boundedStart), static_cast<std::size_t>(boundedLength)));
    };
}

void StandardLibrary::registerArrays() {
    functions_["push"] = [](const std::vector<Value>& args) {
        ArrayValue array = requireArray(args, 0, "push");
        if (args.size() < 2) {
            throw std::runtime_error("push: missing value argument");
        }
        array.elements.push_back(std::make_shared<Value>(args[1]));
        return Value::makeArray(array);
    };

    functions_["pop"] = [](const std::vector<Value>& args) {
        ArrayValue array = requireArray(args, 0, "pop");
        if (!array.elements.empty()) {
            array.elements.pop_back();
        }
        return Value::makeArray(array);
    };

    functions_["map"] = [this](const std::vector<Value>& args) {
        ArrayValue array = requireArray(args, 0, "map");
        const std::string fnName = requireString(args, 1, "map");

        if (!has(fnName)) {
            throw std::runtime_error("map: unknown builtin mapper " + fnName);
        }

        ArrayValue output;
        for (const auto& item : array.elements) {
            output.elements.push_back(std::make_shared<Value>(call(fnName, {*item})));
        }
        return Value::makeArray(output);
    };

    functions_["filter"] = [](const std::vector<Value>& args) {
        ArrayValue array = requireArray(args, 0, "filter");
        const std::string mode = requireString(args, 1, "filter");

        ArrayValue output;
        for (const auto& item : array.elements) {
            bool keep = false;
            if (mode == "truthy") {
                keep = item->type() != TypeKind::Null &&
                       !(item->type() == TypeKind::Boolean && !item->asBoolean());
            } else if (mode == "nonNull") {
                keep = item->type() != TypeKind::Null;
            } else if (mode == "gt") {
                const double threshold = requireNumber(args, 2, "filter");
                keep = item->type() == TypeKind::Number && item->asNumber() > threshold;
            } else if (mode == "lt") {
                const double threshold = requireNumber(args, 2, "filter");
                keep = item->type() == TypeKind::Number && item->asNumber() < threshold;
            } else if (mode == "eq") {
                const double threshold = requireNumber(args, 2, "filter");
                keep = item->type() == TypeKind::Number && item->asNumber() == threshold;
            } else {
                throw std::runtime_error("filter: unsupported mode " + mode);
            }

            if (keep) {
                output.elements.push_back(item);
            }
        }
        return Value::makeArray(output);
    };
}

void StandardLibrary::registerFileIO() {
    functions_["readFile"] = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "readFile");
        std::ifstream file(path);
        if (!file) {
            throw std::runtime_error("readFile: cannot open file " + path);
        }

        std::ostringstream out;
        out << file.rdbuf();
        return Value(out.str());
    };

    functions_["writeFile"] = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "writeFile");
        const std::string content = requireString(args, 1, "writeFile");

        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("writeFile: cannot open file " + path);
        }
        file << content;
        return Value::makeNull();
    };

    functions_["appendFile"] = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "appendFile");
        const std::string content = requireString(args, 1, "appendFile");
        std::ofstream file(path, std::ios::app);
        if (!file) {
            throw std::runtime_error("appendFile: cannot open file " + path);
        }
        file << content;
        return Value::makeNull();
    };

    functions_["fileExists"] = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "fileExists");
        return Value(std::filesystem::exists(path));
    };
}

void StandardLibrary::registerGameGraphics() {
    functions_["canvasCreate"] = [](const std::vector<Value>& args) {
        const double width = requireNumber(args, 0, "canvasCreate");
        const double height = requireNumber(args, 1, "canvasCreate");
        ObjectValue canvas;
        canvas.fields["kind"] = std::make_shared<Value>(Value(std::string("canvas")));
        canvas.fields["width"] = std::make_shared<Value>(Value(width));
        canvas.fields["height"] = std::make_shared<Value>(Value(height));
        return Value::makeObject(canvas);
    };

    functions_["inputIsKeyDown"] = [](const std::vector<Value>& args) {
        (void)requireString(args, 0, "inputIsKeyDown");
        return Value(false);
    };

    functions_["timerNowMs"] = [](const std::vector<Value>&) {
        const auto now = std::chrono::steady_clock::now().time_since_epoch();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
        return Value(static_cast<double>(ms));
    };

    functions_["spriteLoad"] = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "spriteLoad");
        ObjectValue sprite;
        sprite.fields["kind"] = std::make_shared<Value>(Value(std::string("sprite")));
        sprite.fields["path"] = std::make_shared<Value>(Value(path));
        return Value::makeObject(sprite);
    };

    functions_["spriteDraw"] = [](const std::vector<Value>& args) {
        (void)args;
        return Value::makeNull();
    };

    functions_["audioPlay"] = [](const std::vector<Value>& args) {
        (void)requireString(args, 0, "audioPlay");
        return Value::makeNull();
    };
}

void StandardLibrary::registerNetworking() {
    functions_["httpGet"] = [](const std::vector<Value>& args) {
        const std::string url = requireString(args, 0, "httpGet");
        const std::string command = "curl -fsSL \"" + url + "\" 2>/dev/null";
        std::array<char, 256> buffer{};
        std::string result;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("httpGet: failed to spawn curl process");
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result += buffer.data();
        }
        const int rc = pclose(pipe);
        if (rc != 0) {
            throw std::runtime_error("httpGet: curl failed for " + url);
        }
        return Value(result);
    };
}

void StandardLibrary::registerInteroperability() {
    functions_["objectCreate"] = [](const std::vector<Value>&) {
        return Value::makeObject(ObjectValue{});
    };

    functions_["objectSet"] = [](const std::vector<Value>& args) {
        if (args.size() < 3) {
            throw std::runtime_error("objectSet: expected object, key, value");
        }
        ObjectValue object = args[0].asObject();
        const std::string key = args[1].asString();
        object.fields[key] = std::make_shared<Value>(args[2]);
        return Value::makeObject(object);
    };

    functions_["objectGet"] = [](const std::vector<Value>& args) {
        if (args.size() < 2) {
            throw std::runtime_error("objectGet: expected object and key");
        }
        const ObjectValue object = args[0].asObject();
        const std::string key = args[1].asString();
        const auto it = object.fields.find(key);
        if (it == object.fields.end()) {
            return Value::makeNull();
        }
        return *it->second;
    };

    functions_["classCreate"] = [](const std::vector<Value>& args) {
        const std::string name = requireString(args, 0, "classCreate");
        return Value::makeClass(name, ObjectValue{});
    };

    functions_["env"] = [](const std::vector<Value>& args) {
        const std::string name = requireString(args, 0, "env");
        const char* value = std::getenv(name.c_str());
        if (!value) {
            return Value::makeNull();
        }
        return Value(std::string(value));
    };

    functions_["exec"] = [](const std::vector<Value>& args) {
        const std::string command = requireString(args, 0, "exec");
        std::array<char, 256> buffer{};
        std::string result;
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("exec: failed to spawn command");
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            result += buffer.data();
        }
        pclose(pipe);
        return Value(result);
    };
}

} // namespace magphos::runtime
