#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const auto root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream stream(std::filesystem::path(root ? root : ".") / path);
  if (!stream) throw std::runtime_error(std::string("missing focused module: ") + path);
  return {std::istreambuf_iterator<char>(stream), {}};
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
bool has(const std::string& source, std::string_view text) {
  return source.find(text) != std::string::npos;
}
std::size_t lines(const std::string& source) {
  return std::count(source.begin(), source.end(), '\n') + 1;
}
} // namespace

int main() {
  try {
    const auto core_leaf = read("include/cgpui/core/window_title_bar.hpp");
    const auto core_window = read("include/cgpui/core/window.hpp");
    const auto leaf = read("include/cgpui/platform/platform_window_title_bar.hpp");
    const auto platform_window = read("include/cgpui/platform/platform_window.hpp");
    const auto fallback = read("src/platform/platform_window_title_bar.cpp");
    const auto element_leaf = read("include/cgpui/ui/element_window_control_nodes.hpp");
    const auto element = read("src/ui/element_window_control_nodes.cpp");
    const auto paint = read("src/ui/paint_window_control_regions.cpp");
    const auto runtime = read("src/ui/runtime_window_title_bar.cpp");
    const auto runtime_header = read("src/ui/runtime_window_title_bar_internal.hpp");
    const auto facade = read("src/ui/window_title_bar.cpp");
    const auto win32_header = read("src/platform/win32/win32_window_title_bar_internal.hpp");
    const auto win32_hit = read("src/platform/win32/win32_window_title_bar_hit_test.cpp");
    const auto win32 = read("src/platform/win32/win32_window_title_bar.cpp");
    const auto win32_actions = read("src/platform/win32/win32_window_title_bar_actions.cpp");
    const auto win32_proc = read("src/platform/win32/win32_window_proc_title_bar.cpp");
    const auto wayland_state =
        read("src/platform/linux/wayland_window_title_bar_state_internal.hpp");
    const auto wayland_fragment =
        read("src/platform/linux/wayland_window_title_bar_internal.hpp");
    const auto wayland_api = read("src/platform/linux/wayland_window_title_bar_api.hpp");
    const auto wayland_hit = read("src/platform/linux/wayland_window_title_bar_hit.cpp");
    const auto wayland = read("src/platform/linux/wayland_window_title_bar.cpp");
    const auto protocol =
        read("src/platform/linux/wayland_protocol_xdg_toplevel_interactive.cpp");
    const auto protocol_header =
        read("src/platform/linux/wayland_protocol_xdg_toplevel_interactive_internal.hpp");
    const auto registered =
        read("src/platform/linux/wayland_window_registered_title_bar.cpp");
    const auto module = read("build/xmake/capabilities/client_title_bar_targets.lua");
    require(lines(core_leaf) <= 45 && has(core_leaf, "enum class WindowTitleBarMode") &&
            has(core_leaf, "enum class WindowControlArea") &&
            has(core_leaf, "struct WindowControlRegion") &&
            has(core_window, "WindowTitleBarMode title_bar = WindowTitleBarMode::system;"),
            "title bar vocabulary is a core leaf carried by WindowChromeOptions");
    require(lines(leaf) <= 60 && !has(leaf, "<windows.h>") && !has(leaf, "wayland") &&
            has(leaf, "struct PlatformWindowFrameMetrics") &&
            has(leaf, "enum class PlatformWindowAction") &&
            has(platform_window, "set_window_control_regions(") &&
            has(platform_window, "frame_metrics() const;") &&
            has(platform_window, "perform_window_action(") &&
            lines(fallback) <= 65 && has(fallback, "request_display_state(") &&
            has(fallback, ".supported = false"),
            "frame metrics and actions are a neutral leaf; the fallback reuses display state");
    require(lines(element_leaf) <= 40 && lines(element) <= 70 && lines(paint) <= 30 &&
            has(element, "add_window_control_region(") &&
            lines(runtime) <= 30 && lines(runtime_header) <= 25 && lines(facade) <= 30 &&
            has(runtime, "set_window_control_regions(") &&
            has(facade, "Window::perform_window_action("),
            "element, paint, runtime and facade parts are focused sources");
    require(lines(win32_header) <= 80 && lines(win32_hit) <= 90 && lines(win32) <= 125 &&
            lines(win32_actions) <= 105 && lines(win32_proc) <= 25 &&
            has(win32, "WM_NCCALCSIZE") && has(win32, "WM_NCHITTEST") &&
            has(win32, "WM_NCMOUSELEAVE") && has(win32, "TME_NONCLIENT") &&
            has(win32_hit, "HTMAXBUTTON") && has(win32_hit, "SM_CXPADDEDBORDER") &&
            has(win32_hit, "GetSystemMetricsForDpi("),
            "Win32 client title bar is split into hit test, messages and actions");
    require(lines(wayland_state) <= 45 && lines(wayland_fragment) <= 12 &&
            lines(wayland_api) <= 20 && lines(wayland_hit) <= 70 &&
            lines(wayland) <= 140 && lines(protocol) <= 40 &&
            lines(protocol_header) <= 30 && has(wayland, "xdg_toplevel_move(") &&
            has(wayland, "xdg_toplevel_resize(") &&
            has(wayland, "xdg_toplevel_show_window_menu(") && lines(registered) <= 25 &&
            has(registered, "RegisteredWaylandWindow::frame_metrics("),
            "Wayland client title bar and its protocol requests are focused modules");
    require(has(module, "tests/platform/win32_client_title_bar_test.cpp") &&
            has(module, "tests/platform/wayland_client_title_bar_test.cpp") &&
            has(module, "tests/ui/window_control_region_test.cpp") &&
            has(module, "is_plat(\"windows\")") && has(module, "is_plat(\"linux\")"),
            "client title bar tests live in the capability module, guarded by platform");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Client title bar structure failed: " << error.what() << '\n';
    return 1;
  }
}
