#pragma once

namespace cgpui { class Renderer; class PlatformWindow; }
namespace cgpui_test {
bool dynamic_client_paint(cgpui::Renderer&, const cgpui::PlatformWindow&, bool blue, bool alpha);
bool dynamic_client_resize(cgpui::Renderer&, cgpui::PlatformWindow&);
} // namespace cgpui_test
