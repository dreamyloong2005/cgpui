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
    const auto core_leaf = read("include/cgpui/core/window_backdrop.hpp");
    const auto core_window = read("include/cgpui/core/window.hpp");
    const auto leaf = read("include/cgpui/platform/platform_window_backdrop.hpp");
    const auto platform_window = read("include/cgpui/platform/platform_window.hpp");
    const auto composition = read("include/cgpui/renderer/renderer_surface_composition.hpp");
    const auto fallback = read("src/platform/platform_window_backdrop.cpp");
    const auto renderer_fallback = read("src/renderer/renderer_surface_composition.cpp");
    const auto vulkan = read("src/renderer/vulkan/vulkan_surface_composition.cpp");
    const auto vulkan_header =
        read("src/renderer/vulkan/vulkan_surface_composition_internal.hpp");
    const auto selection = read("src/renderer/vulkan/vulkan_surface_selection.cpp");
    const auto win32_header = read("src/platform/win32/win32_window_backdrop_internal.hpp");
    const auto win32_plan = read("src/platform/win32/win32_window_backdrop_plan.cpp");
    const auto win32 = read("src/platform/win32/win32_window_backdrop.cpp");
    const auto win32_window = read("src/platform/win32/win32_window_backdrop_window.cpp");
    const auto monitor = read("src/platform/win32/win32_appearance_monitor.cpp");
    const auto wayland_header = read("src/platform/linux/wayland_backdrop_internal.hpp");
    const auto wayland_globals = read("src/platform/linux/wayland_backdrop_globals.cpp");
    const auto wayland_window = read("src/platform/linux/wayland_window_backdrop.cpp");
    const auto wayland_fragment =
        read("src/platform/linux/wayland_window_backdrop_internal.hpp");
    const auto protocol =
        read("src/platform/linux/wayland_protocol_background_effect.cpp");
    const auto protocol_header =
        read("src/platform/linux/wayland_protocol_background_effect_internal.hpp");
    const auto protocol_tables =
        read("src/platform/linux/wayland_protocol_background_effect_interfaces.cpp");
    const auto runtime = read("src/ui/runtime_window_backdrop.cpp");
    const auto runtime_header = read("src/ui/runtime_window_backdrop_internal.hpp");
    const auto facade = read("src/ui/window_backdrop.cpp");
    const auto module = read("build/xmake/capabilities/window_backdrop_targets.lua");
    require(lines(core_leaf) <= 20 && has(core_leaf, "enum class WindowBackdrop") &&
            has(core_window, "WindowBackdrop backdrop = WindowBackdrop::none;"),
            "the backdrop enum is a core leaf carried by WindowDescriptor");
    require(lines(leaf) <= 35 && !has(leaf, "<windows.h>") && !has(leaf, "wayland") &&
            has(leaf, "struct PlatformWindowBackdropState") &&
            has(leaf, "bool transparent_surface") &&
            has(platform_window, "apply_window_backdrop(") &&
            has(platform_window, "window_backdrop_state() const;"),
            "backdrop state is a platform-neutral leaf on PlatformWindow");
    require(lines(composition) <= 40 && !has(composition, "vulkan") &&
            lines(fallback) <= 30 && has(fallback, ".supported = false") &&
            lines(renderer_fallback) <= 25,
            "fallbacks report unsupported and the composition leaf stays neutral");
    require(lines(vulkan) <= 70 && lines(vulkan_header) <= 25 &&
            has(selection,
                "transparent_preference{\n"
                "      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,\n"
                "      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,\n"
                "      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,\n  };"),
            "Vulkan composition reporting is focused; transparent order is pre, inherit, opaque");
    require(lines(win32_header) <= 90 && lines(win32_plan) <= 100 &&
            lines(win32) <= 110 && lines(win32_window) <= 20 &&
            has(win32_plan, "RtlGetVersion") && !has(win32_plan, "GetVersionEx") &&
            has(win32, "DwmExtendFrameIntoClientArea(") &&
            has(win32, "DwmSetWindowAttribute(") &&
            !has(win32, "SetLayeredWindowAttributes") &&
            has(win32_plan, "WS_EX_LAYERED") &&
            has(monitor, "win32_reevaluate_thread_window_backdrops("),
            "Win32 backdrop is DWM-only, uses RtlGetVersion, and follows the monitor");
    require(lines(wayland_header) <= 95 && lines(wayland_globals) <= 130 &&
            lines(wayland_window) <= 115 && lines(wayland_fragment) <= 8 &&
            lines(protocol) <= 100 && lines(protocol_header) <= 70 &&
            lines(protocol_tables) <= 60 &&
            has(protocol_tables, "\"ext_background_effect_manager_v1\"") &&
            has(protocol_tables, "\"org_kde_kwin_blur_manager\""),
            "Wayland backdrop and its protocol bindings are focused modules");
    require(lines(runtime) <= 60 && lines(runtime_header) <= 50 &&
            lines(facade) <= 50 && has(facade, "Window::set_backdrop(") &&
            has(runtime, "set_transparent_surface(true)"),
            "runtime and Window facade forward through focused sources");
    require(has(module, "tests/platform/win32_window_backdrop_test.cpp") &&
            has(module, "tests/platform/wayland_window_backdrop_test.cpp") &&
            has(module, "tests/renderer/vulkan_surface_composition_test.cpp") &&
            has(module, "is_plat(\"windows\")") && has(module, "is_plat(\"linux\")"),
            "backdrop tests live in the capability module, guarded by platform");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Window backdrop structure failed: " << error.what() << '\n';
    return 1;
  }
}
