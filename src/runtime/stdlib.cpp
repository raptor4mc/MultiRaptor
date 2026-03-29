#include "runtime/stdlib.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <future>
#include <mutex>
#include <optional>
#include <queue>
#include <regex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

namespace magphos::runtime {

namespace {

struct SemaphoreState {
    std::mutex mu;
    std::condition_variable cv;
    int count = 0;
};

struct ChannelState {
    std::mutex mu;
    std::condition_variable cv;
    std::queue<Value> queue;
};

std::mutex gConcurrencyMu;
int gNextHandle = 1;
std::unordered_map<int, std::future<Value>> gTasks;
std::unordered_map<int, std::shared_ptr<std::mutex>> gMutexes;
std::unordered_map<int, std::shared_ptr<SemaphoreState>> gSemaphores;
std::unordered_map<int, std::shared_ptr<ChannelState>> gChannels;
std::unordered_map<int, int> gSockets;

int allocateHandle() {
    std::lock_guard<std::mutex> lock(gConcurrencyMu);
    return gNextHandle++;
}

ObjectValue makeHandleObject(const std::string& kind, int id) {
    ObjectValue object;
    object.fields["kind"] = std::make_shared<Value>(Value(kind));
    object.fields["id"] = std::make_shared<Value>(Value(static_cast<double>(id)));
    return object;
}

int requireHandleId(const Value& value, const std::string& expectedKind, const std::string& fn) {
    const ObjectValue object = value.asObject();
    const auto kindIt = object.fields.find("kind");
    const auto idIt = object.fields.find("id");
    if (kindIt == object.fields.end() || idIt == object.fields.end()) {
        throw std::runtime_error(fn + ": malformed handle object");
    }
    if (kindIt->second->asString() != expectedKind) {
        throw std::runtime_error(fn + ": expected handle kind '" + expectedKind + "'");
    }
    return static_cast<int>(idIt->second->asNumber());
}

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
    registerAdvancedMath();
    registerStrings();
    registerStringExtras();
    registerArrays();
    registerFileIO();
    registerGameGraphics();
    registerNetworking();
    registerInteroperability();
    registerConcurrency();
    registerSockets();
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

void StandardLibrary::registerAdvancedMath() {
    functions_["tan"] = [](const std::vector<Value>& args) { return Value(std::tan(requireNumber(args, 0, "tan"))); };
    functions_["asin"] = [](const std::vector<Value>& args) { return Value(std::asin(requireNumber(args, 0, "asin"))); };
    functions_["acos"] = [](const std::vector<Value>& args) { return Value(std::acos(requireNumber(args, 0, "acos"))); };
    functions_["atan"] = [](const std::vector<Value>& args) { return Value(std::atan(requireNumber(args, 0, "atan"))); };
    functions_["log"] = [](const std::vector<Value>& args) { return Value(std::log10(requireNumber(args, 0, "log"))); };
    functions_["ln"] = [](const std::vector<Value>& args) { return Value(std::log(requireNumber(args, 0, "ln"))); };
    functions_["exp"] = [](const std::vector<Value>& args) { return Value(std::exp(requireNumber(args, 0, "exp"))); };
    functions_["pow"] = [](const std::vector<Value>& args) {
        return Value(std::pow(requireNumber(args, 0, "pow"), requireNumber(args, 1, "pow")));
    };
    functions_["floor"] = [](const std::vector<Value>& args) { return Value(std::floor(requireNumber(args, 0, "floor"))); };
    functions_["ceil"] = [](const std::vector<Value>& args) { return Value(std::ceil(requireNumber(args, 0, "ceil"))); };
    functions_["round"] = [](const std::vector<Value>& args) { return Value(std::round(requireNumber(args, 0, "round"))); };
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

void StandardLibrary::registerStringExtras() {
    functions_["join"] = [](const std::vector<Value>& args) {
        ArrayValue array = requireArray(args, 0, "join");
        const std::string delimiter = requireString(args, 1, "join");
        std::ostringstream out;
        for (std::size_t i = 0; i < array.elements.size(); ++i) {
            if (i > 0) out << delimiter;
            out << array.elements[i]->asString();
        }
        return Value(out.str());
    };

    functions_["regexMatch"] = [](const std::vector<Value>& args) {
        const std::string input = requireString(args, 0, "regexMatch");
        const std::string pattern = requireString(args, 1, "regexMatch");
        return Value(std::regex_search(input, std::regex(pattern)));
    };

    functions_["regexReplace"] = [](const std::vector<Value>& args) {
        const std::string input = requireString(args, 0, "regexReplace");
        const std::string pattern = requireString(args, 1, "regexReplace");
        const std::string replacement = requireString(args, 2, "regexReplace");
        return Value(std::regex_replace(input, std::regex(pattern), replacement));
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
    const auto readImpl = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "readFile");
        std::ifstream file(path);
        if (!file) {
            throw std::runtime_error("readFile: cannot open file " + path);
        }

        std::ostringstream out;
        out << file.rdbuf();
        return Value(out.str());
    };
    functions_["readFile"] = readImpl;
    functions_["read"] = readImpl;

    const auto writeImpl = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "writeFile");
        const std::string content = requireString(args, 1, "writeFile");

        std::ofstream file(path);
        if (!file) {
            throw std::runtime_error("writeFile: cannot open file " + path);
        }
        file << content;
        return Value::makeNull();
    };
    functions_["writeFile"] = writeImpl;
    functions_["write"] = writeImpl;

    const auto appendImpl = [](const std::vector<Value>& args) {
        const std::string path = requireString(args, 0, "appendFile");
        const std::string content = requireString(args, 1, "appendFile");
        std::ofstream file(path, std::ios::app);
        if (!file) {
            throw std::runtime_error("appendFile: cannot open file " + path);
        }
        file << content;
        return Value::makeNull();
    };
    functions_["appendFile"] = appendImpl;
    functions_["append"] = appendImpl;

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

void StandardLibrary::registerConcurrency() {
    functions_["threadSpawn"] = [](const std::vector<Value>& args) {
        const double delayMs = requireNumber(args, 0, "threadSpawn");
        const Value value = args.size() >= 2 ? args[1] : Value::makeNull();
        const int handle = allocateHandle();
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            gTasks[handle] = std::async(std::launch::async, [delayMs, value]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delayMs)));
                return value;
            });
        }
        return Value::makeObject(makeHandleObject("thread", handle));
    };

    functions_["threadAwait"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "thread", "threadAwait");
        std::future<Value> task;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            auto it = gTasks.find(handle);
            if (it == gTasks.end()) {
                throw std::runtime_error("threadAwait: unknown thread handle");
            }
            task = std::move(it->second);
            gTasks.erase(it);
        }
        return task.get();
    };

    functions_["mutexCreate"] = [](const std::vector<Value>&) {
        const int handle = allocateHandle();
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            gMutexes[handle] = std::make_shared<std::mutex>();
        }
        return Value::makeObject(makeHandleObject("mutex", handle));
    };

    functions_["mutexLock"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "mutex", "mutexLock");
        std::lock_guard<std::mutex> lock(gConcurrencyMu);
        auto it = gMutexes.find(handle);
        if (it == gMutexes.end()) throw std::runtime_error("mutexLock: unknown mutex");
        it->second->lock();
        return Value::makeNull();
    };

    functions_["mutexUnlock"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "mutex", "mutexUnlock");
        std::lock_guard<std::mutex> lock(gConcurrencyMu);
        auto it = gMutexes.find(handle);
        if (it == gMutexes.end()) throw std::runtime_error("mutexUnlock: unknown mutex");
        it->second->unlock();
        return Value::makeNull();
    };

    functions_["semaphoreCreate"] = [](const std::vector<Value>& args) {
        const int initial = static_cast<int>(requireNumber(args, 0, "semaphoreCreate"));
        const int handle = allocateHandle();
        auto sem = std::make_shared<SemaphoreState>();
        sem->count = initial;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            gSemaphores[handle] = sem;
        }
        return Value::makeObject(makeHandleObject("semaphore", handle));
    };

    functions_["semaphoreAcquire"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "semaphore", "semaphoreAcquire");
        std::shared_ptr<SemaphoreState> sem;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            sem = gSemaphores.at(handle);
        }
        std::unique_lock<std::mutex> lk(sem->mu);
        sem->cv.wait(lk, [&]() { return sem->count > 0; });
        sem->count -= 1;
        return Value::makeNull();
    };

    functions_["semaphoreRelease"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "semaphore", "semaphoreRelease");
        std::shared_ptr<SemaphoreState> sem;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            sem = gSemaphores.at(handle);
        }
        {
            std::lock_guard<std::mutex> lk(sem->mu);
            sem->count += 1;
        }
        sem->cv.notify_one();
        return Value::makeNull();
    };

    functions_["channelCreate"] = [](const std::vector<Value>&) {
        const int handle = allocateHandle();
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            gChannels[handle] = std::make_shared<ChannelState>();
        }
        return Value::makeObject(makeHandleObject("channel", handle));
    };

    functions_["channelSend"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "channel", "channelSend");
        std::shared_ptr<ChannelState> channel;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            channel = gChannels.at(handle);
        }
        {
            std::lock_guard<std::mutex> lk(channel->mu);
            channel->queue.push(args.at(1));
        }
        channel->cv.notify_one();
        return Value::makeNull();
    };

    functions_["channelRecv"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "channel", "channelRecv");
        std::shared_ptr<ChannelState> channel;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            channel = gChannels.at(handle);
        }
        std::unique_lock<std::mutex> lk(channel->mu);
        channel->cv.wait(lk, [&]() { return !channel->queue.empty(); });
        Value value = channel->queue.front();
        channel->queue.pop();
        return value;
    };
}

void StandardLibrary::registerSockets() {
    functions_["tcpConnect"] = [](const std::vector<Value>& args) {
        const std::string host = requireString(args, 0, "tcpConnect");
        const int port = static_cast<int>(requireNumber(args, 1, "tcpConnect"));
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) throw std::runtime_error("tcpConnect: socket creation failed");

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<uint16_t>(port));
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) <= 0) {
            ::close(fd);
            throw std::runtime_error("tcpConnect: invalid IPv4 host");
        }
        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(fd);
            throw std::runtime_error("tcpConnect: connect failed");
        }

        const int handle = allocateHandle();
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            gSockets[handle] = fd;
        }
        return Value::makeObject(makeHandleObject("socket", handle));
    };

    functions_["socketSend"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "socket", "socketSend");
        const std::string data = requireString(args, 1, "socketSend");
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            fd = gSockets.at(handle);
        }
        const auto sent = ::send(fd, data.data(), data.size(), 0);
        return Value(static_cast<double>(sent));
    };

    functions_["socketRecv"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "socket", "socketRecv");
        const int maxBytes = static_cast<int>(requireNumber(args, 1, "socketRecv"));
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            fd = gSockets.at(handle);
        }
        std::string buffer(static_cast<std::size_t>(maxBytes), '\0');
        const auto n = ::recv(fd, buffer.data(), buffer.size(), 0);
        if (n <= 0) return Value(std::string(""));
        buffer.resize(static_cast<std::size_t>(n));
        return Value(buffer);
    };

    functions_["socketClose"] = [](const std::vector<Value>& args) {
        const int handle = requireHandleId(args.at(0), "socket", "socketClose");
        int fd = -1;
        {
            std::lock_guard<std::mutex> lock(gConcurrencyMu);
            auto it = gSockets.find(handle);
            if (it == gSockets.end()) return Value::makeNull();
            fd = it->second;
            gSockets.erase(it);
        }
        ::close(fd);
        return Value::makeNull();
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
