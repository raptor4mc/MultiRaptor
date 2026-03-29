#include "web/htmlxd_preview.h"

#include "htmlxd/dom.h"

namespace magphos::web {

using magphos::htmlxd::Attribute;
using magphos::htmlxd::Node;

std::string renderPreviewShellHtml(const std::string& runOutput, bool forceGameView) {
    std::string outputOverlay;
    if (!runOutput.empty()) {
        outputOverlay = "<div style=\"position:fixed;left:12px;bottom:12px;max-width:60ch;background:rgba(15,23,42,0.85);"
                        "border:1px solid rgba(100,116,139,0.6);padding:10px 12px;border-radius:8px;color:#e2e8f0;"
                        "font-family:system-ui,sans-serif\"><strong>Program output</strong><pre style=\"margin:6px 0 0;"
                        "white-space:pre-wrap\">" + magphos::htmlxd::escapeHtml(runOutput) + "</pre></div>";
    }

    Node hud = Node::element("div",
                             {Attribute{"id", "hud"}},
                             {
                                 Node::element("strong", {}, {Node::text("MagPhos Preview")}),
                                 Node::raw("<br />"),
                                 Node::text("Add preview/index.html in your project to replace this.")
                             });

    if (forceGameView) {
        hud.attributes.push_back({"style", "left:auto;right:12px"});
    }

    Node body = Node::element("body", {}, {
        Node::element("canvas", {Attribute{"id", "screen"}}, {}),
        hud,
        outputOverlay.empty() ? Node::text("") : Node::raw(outputOverlay),
        Node::element("script", {}, {
            Node::raw(R"JS(
const canvas = document.getElementById('screen');
const ctx = canvas.getContext('2d');
let t = 0;
function resize() {
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;
}
function draw() {
  t += 0.02;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.fillStyle = '#22d3ee';
  const x = canvas.width * (0.5 + Math.cos(t) * 0.28);
  const y = canvas.height * (0.5 + Math.sin(t * 1.7) * 0.2);
  ctx.beginPath();
  ctx.arc(x, y, 28, 0, Math.PI * 2);
  ctx.fill();
  requestAnimationFrame(draw);
}
window.addEventListener('resize', resize);
resize();
draw();
)JS")
        })
    });

    Node root = Node::raw(
        "<!doctype html>" +
        magphos::htmlxd::render(
            Node::element("html", {}, {
                Node::element("head", {}, {
                    Node::element("meta", {Attribute{"charset", "utf-8"}}, {}),
                    Node::element("style", {}, {Node::raw(
                        "html, body { margin: 0; width: 100%; height: 100%; background: #0f172a; color: #e2e8f0; font-family: system-ui, sans-serif; }"
                        "#hud { position: fixed; inset: 12px auto auto 12px; background: rgba(15, 23, 42, 0.85); border: 1px solid rgba(100, 116, 139, 0.6); padding: 10px 12px; border-radius: 8px; }"
                        "canvas { display: block; width: 100vw; height: 100vh; background: radial-gradient(circle at 40% 30%, #1d4ed8, #0f172a 58%); }"
                    )})
                }),
                body
            })
        )
    );

    return root.content;
}

} // namespace magphos::web
