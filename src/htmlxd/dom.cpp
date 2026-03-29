#include "htmlxd/dom.h"

namespace magphos::htmlxd {

Node Node::text(std::string value) {
    Node node;
    node.kind = Kind::Text;
    node.content = std::move(value);
    return node;
}

Node Node::raw(std::string value) {
    Node node;
    node.kind = Kind::Raw;
    node.content = std::move(value);
    return node;
}

Node Node::element(std::string tag,
                   std::vector<Attribute> attributes,
                   std::vector<Node> children) {
    Node node;
    node.kind = Kind::Element;
    node.tag = std::move(tag);
    node.attributes = std::move(attributes);
    node.children = std::move(children);
    return node;
}

std::string escapeHtml(const std::string& value) {
    std::string out;
    out.reserve(value.size());
    for (const char ch : value) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(ch); break;
        }
    }
    return out;
}

std::string render(const Node& node) {
    if (node.kind == Node::Kind::Text) {
        return escapeHtml(node.content);
    }
    if (node.kind == Node::Kind::Raw) {
        return node.content;
    }

    std::string out;
    out += "<" + node.tag;
    for (const auto& attr : node.attributes) {
        out += " " + attr.name + "=\"" + escapeHtml(attr.value) + "\"";
    }
    out += ">";
    for (const auto& child : node.children) {
        out += render(child);
    }
    out += "</" + node.tag + ">";
    return out;
}

} // namespace magphos::htmlxd
