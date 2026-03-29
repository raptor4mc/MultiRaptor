#pragma once

#include <string>
#include <utility>
#include <vector>

namespace magphos::htmlxd {

struct Attribute {
    std::string name;
    std::string value;
};

struct Node {
    enum class Kind {
        Element,
        Text,
        Raw
    };

    Kind kind = Kind::Text;
    std::string tag;
    std::vector<Attribute> attributes;
    std::vector<Node> children;
    std::string content;

    static Node text(std::string value);
    static Node raw(std::string value);
    static Node element(std::string tag,
                        std::vector<Attribute> attributes = {},
                        std::vector<Node> children = {});
};

std::string escapeHtml(const std::string& value);
std::string render(const Node& node);

} // namespace magphos::htmlxd
