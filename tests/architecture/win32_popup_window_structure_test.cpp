#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const auto root = CGPUI_SOURCE_ROOT;
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

// Win32 popup windows (WindowKind::popup) live in their own module: the
// window class keeps their state, so Win32Window only declares place_popup
// (docs/superpowers/plans/2026-11-22-popup-windows-windows-linux-follow-up.md).
int main() {
  try {
    struct Source {
      const char* path;
      std::size_t max_lines;
    };
    constexpr Source sources[]{
        {"src/platform/win32/win32_popup_window_internal.hpp", 60},
        {"src/platform/win32/win32_popup_window.cpp", 90},
        {"src/platform/win32/win32_popup_placement.cpp", 120},
        {"src/platform/win32/win32_window_proc_popup_internal.hpp", 20},
        {"src/platform/win32/win32_window_proc_popup.cpp", 30},
    };
    for (const Source& source : sources) {
      require(lines(read(source.path)) <= source.max_lines, source.path);
    }
    const auto header = read("src/platform/win32/win32_popup_window_internal.hpp");
    const auto window = read("src/platform/win32/win32_popup_window.cpp");
    const auto placement = read("src/platform/win32/win32_popup_placement.cpp");
    const auto proc = read("src/platform/win32/win32_window_proc_popup.cpp");
    require(has(header, "WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP") &&
                has(header, "win32_popup_style = WS_POPUP") &&
                has(window, "SW_SHOWNOACTIVATE") &&
                has(window, "win32_set_logical_client_size(hwnd, descriptor.size)") &&
                has(window, "a popup window needs a parent window") &&
                has(window, "owner, nullptr, instance"),
            "popups are owned, unframed, never activated and have no redirection surface");
    require(has(header, "win32_popup_pointer_transparent_style =") &&
                has(header, "WS_EX_LAYERED | WS_EX_TRANSPARENT") &&
                has(window, "SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA)") &&
                !has(proc, "HTTRANSPARENT"),
            "pointer-transparent popups are layered and transparent (HTTRANSPARENT "
            "reaches same-thread windows only)");
    require(has(placement, "platform_popup_frame(") && has(placement, "MonitorFromPoint(") &&
                has(placement, "GetMonitorInfoW(") && has(placement, "rcWork") &&
                has(placement, "ClientToScreen(") && has(placement, "GetDpiForWindow(owner)") &&
                has(placement, "SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER") &&
                has(placement, "EnumThreadWindows("),
            "placement uses the shared math on the anchor monitor's work area");
    require(has(proc, "MA_NOACTIVATE") && has(proc, "win32_move_owned_popups(hwnd)") &&
                has(read("src/platform/win32/win32_window_proc.cpp"),
                    "win32_window_proc_handle_popup(hwnd, message, result)"),
            "the window procedure keeps popups inactive and moves them with their owner");
    const auto win32_window = read("src/platform/win32/win32_window_internal.hpp");
    const auto native_surface = read("src/platform/win32/win32_window.cpp");
    const auto application = read("src/platform/win32/win32_application.cpp");
    const auto system = read("src/platform/win32/win32_system_application.cpp");
    require(has(win32_window, "place_popup(WindowPopupPlacement placement) override") &&
                has(native_surface, ".composition = win32_is_popup_window(hwnd_)") &&
                has(application, "create_win32_popup_window(") &&
                has(application, "windows_.add(") &&
                application.find("if (popup) return") < application.find("windows_.add(") &&
                has(system, "win32_composition_host_available()"),
            "popups mark their surface, skip the menu bar and window list, and are "
            "supported where the composition probe passes");
    const auto package = read("packages/platform-win32/mcpp.toml");
    require(!has(package, "cgpui.renderer-win32-composition") &&
                has(package, "dcomp.lib") && !has(system, "renderer/"),
            "the platform owns its host probe without a renderer dependency");
    const auto platform_tests = read("tests/platform/mcpp.toml");
    const auto behavior = read("tests/platform/win32_popup_window_test.cpp");
    require(platform_tests.find("\"win32_popup_window_test.cpp\"") >
                    platform_tests.find("[target.'cfg(os = \"windows\")'.test]") &&
                has(behavior, "GW_OWNER") && has(behavior, "flipped") &&
                has(behavior, "WindowFromPoint") && has(behavior, "WM_MOUSEACTIVATE"),
            "a Win32 platform test covers the popup cases of the macOS one");
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
