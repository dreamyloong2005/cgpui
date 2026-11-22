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

int main() {
  try {
    const auto leaf = read("include/cgpui/platform/platform_appearance.hpp");
    const auto application = read("include/cgpui/platform/platform_application.hpp");
    const auto aggregate = read("include/cgpui/platform/platform.hpp");
    const auto event_platform = read("include/cgpui/core/event_platform.hpp");
    const auto fallback = read("src/platform/platform_appearance.cpp");
    const auto reader = read("src/platform/win32/win32_system_appearance.cpp");
    const auto reader_header = read("src/platform/win32/win32_system_appearance_internal.hpp");
    const auto monitor = read("src/platform/win32/win32_appearance_monitor.cpp");
    const auto monitor_header = read("src/platform/win32/win32_appearance_monitor_internal.hpp");
    const auto win32_application = read("src/platform/win32/win32_application.cpp");
    const auto system_application = read("src/platform/win32/win32_system_application.cpp");
    const auto system_application_header =
        read("src/platform/win32/win32_system_application_internal.hpp");
    const auto factory = read("src/platform/win32/win32_window_factory.cpp");
    const auto runtime = read("src/ui/runtime_system_appearance.cpp");
    const auto runtime_state = read("src/ui/runtime_system_appearance_internal.hpp");
    const auto runtime_declarations =
        read("include/cgpui/ui/detail/window_runtime_system_appearance_declarations.hpp");
    const auto platform_state = read("src/ui/runtime_state_platform_internal.hpp");
    const auto shutdown = read("src/ui/runtime_shutdown.cpp");
    const auto app = read("src/app/app_appearance.cpp");
    const auto window = read("src/ui/window_appearance.cpp");
    const auto module = read("packages/platform-win32/mcpp.toml") +
      read("tests/platform/mcpp.toml") +
      read("tests/ui/mcpp.toml");
    require(lines(leaf) <= 60 && !has(leaf, "<windows.h>") && !has(leaf, "HKEY") &&
            has(leaf, "struct SystemAppearance") && has(leaf, "SystemAppearanceCallback"),
            "appearance leaf must stay small and platform neutral");
    require(has(application, "SystemAppearance system_appearance() const") &&
            has(application, "set_system_appearance_callback(") &&
            has(aggregate, "cgpui/platform/platform_appearance.hpp"),
            "PlatformApplication must expose the appearance query and callback");
    require(!has(event_platform, "Appearance"),
            "appearance is application-level; PlatformEvent must not grow an alternative");
    require(lines(fallback) <= 40 && has(fallback, ".supported = false") &&
            has(fallback, "SystemAppearanceMode::light"),
            "fallback must report an unsupported light snapshot");
    require(lines(reader) <= 130 && lines(reader_header) <= 35 &&
            has(reader, "AppsUseLightTheme") && has(reader, "EnableTransparency") &&
            has(reader, "DwmGetColorizationColor(") &&
            has(reader, "SPI_GETCLIENTAREAANIMATION") && has(reader, "SPI_GETHIGHCONTRAST") &&
            has(reader, "ImmersiveColorSet") && has(reader, "WM_DWMCOLORIZATIONCOLORCHANGED") &&
            has(reader, "DwmSetWindowAttribute(") && !has(reader, "winrt"),
            "win32 appearance reader must use registry, DWM, and SPI sources only");
    require(lines(monitor) <= 90 && lines(monitor_header) <= 45 &&
            has(monitor, "CreateWindowExW(") && !has(monitor, "HWND_MESSAGE") &&
            has(monitor, "win32_apply_title_bar_appearance_to_thread_windows("),
            "monitor must be a hidden top-level window that refreshes title bars");
    require(has(win32_application, "public Win32SystemApplication") &&
            has(system_application_header, "Win32AppearanceMonitor appearance_monitor_") &&
            lines(system_application) <= 40 && lines(system_application_header) <= 35 &&
            !has(win32_application, "WM_SETTINGCHANGE") &&
            !has(win32_application, "RegGetValueW") &&
            has(factory, "win32_apply_immersive_dark_mode("),
            "application and factory must delegate to the focused appearance modules");
    require(lines(runtime) <= 60 && lines(runtime_state) <= 5 &&
            lines(runtime_declarations) <= 5 &&
            has(runtime_declarations, "handle_system_appearance_change(") &&
            has(platform_state, "runtime_system_appearance_internal.hpp") &&
            has(runtime, "PlatformDiagnosticKind::appearance") &&
            has(shutdown, "set_system_appearance_callback({})"),
            "runtime bridge must be focused and detach on shutdown");
    require(lines(app) <= 30 && lines(window) <= 20 &&
            has(app, "App::set_system_appearance_callback(") &&
            has(window, "Window::appearance()"),
            "App and Window facades must forward through focused sources");
    require(has(module, "[targets.cgpui_platform_win32]") &&
            has(module, "\"dwmapi.lib\"") &&
            has(module, "\"win32_system_appearance_test.cpp\"") &&
            has(module, "\"window_runtime_system_appearance_test.cpp\""),
            "dwmapi link and appearance tests must live in the capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "System appearance structure failed: " << error.what() << '\n';
    return 1;
  }
}
