#include "web/source_preview.h"
#include <cctype>

namespace {

bool startsWithIgnoreCase(const std::string& input, const std::string& prefix) {
    if (input.size() < prefix.size()) return false;
    for (size_t i = 0; i < prefix.size(); ++i) {
        char left = static_cast<char>(std::tolower(static_cast<unsigned char>(input[i])));
        char right = static_cast<char>(std::tolower(static_cast<unsigned char>(prefix[i])));
        if (left != right) return false;
    }
    return true;
}

std::string trimLeft(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }
    return input.substr(start);
}

std::string renderMagphosMarkupToHtml(const std::string& source) {
    std::string raw = source;
    std::string trimmed = trimLeft(raw);
    if (!startsWithIgnoreCase(trimmed, "<!DOCTYPE MAGPHOS>")) return "";

    const std::string openTag = "<magphos";
    const std::string closeTag = "</magphos>";

    auto openPos = trimmed.find(openTag);
    auto closePos = trimmed.find(closeTag);
    if (openPos == std::string::npos || closePos == std::string::npos) return "";

    std::string out = raw;
    auto doctypePos = out.find("<!DOCTYPE MAGPHOS>");
    if (doctypePos == std::string::npos) {
        doctypePos = out.find("<!doctype magphos>");
    }
    if (doctypePos != std::string::npos) {
        out.replace(doctypePos, std::string("<!DOCTYPE MAGPHOS>").size(), "<!DOCTYPE html>");
    }

    auto tagOpenPos = out.find("<magphos");
    if (tagOpenPos == std::string::npos) return "";
    out.replace(tagOpenPos, std::string("<magphos").size(), "<html");

    auto tagClosePos = out.find("</magphos>");
    if (tagClosePos == std::string::npos) return "";
    out.replace(tagClosePos, std::string("</magphos>").size(), "</html>");
    return out;
}

std::string renderCssxdToPreviewHtml(const std::string& source) {
    std::string trimmed = trimLeft(source);
    const bool isCssxd =
        startsWithIgnoreCase(trimmed, ":root {") ||
        startsWithIgnoreCase(trimmed, "body {") ||
        startsWithIgnoreCase(trimmed, "* {");
    if (!isCssxd) return "";

    return "<!doctype html><html><head><meta charset=\"utf-8\" /><style>" + source +
           "</style></head><body><main style=\"padding:24px;\"><h1>CSSXD Preview</h1>"
           "<p>This preview is generated from CSSXD-in-Mp source.</p><button>Preview Button</button>"
           "</main></body></html>";
}

} // namespace

namespace magphos::web {

std::string renderPreviewFromSource(const std::string& source) {
    const std::string magphos = renderMagphosMarkupToHtml(source);
    if (!magphos.empty()) return magphos;

    return renderCssxdToPreviewHtml(source);
}

} // namespace magphos::web
