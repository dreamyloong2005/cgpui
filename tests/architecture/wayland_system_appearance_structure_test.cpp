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
  return std::count(source.begin(), source.end(), '\n');
}
bool platform_neutral(const std::string& header) {
  return !has(header, "dbus") && !has(header, "DBus") && !has(header, "wayland") &&
         !has(header, "wl_");
}
} // namespace

int main() {
  try {
    const auto appearance = read("include/cgpui/platform/platform_appearance.hpp");
    const auto dpi = read("include/cgpui/platform/platform_dpi_awareness.hpp");
    const auto application = read("include/cgpui/platform/platform_application.hpp");
    const auto source = read("src/platform/linux/wayland_event_source_internal.hpp");
    const auto loop = read("src/platform/linux/wayland_event_loop.cpp");
    const auto lifecycle = read("src/platform/linux/wayland_application_lifecycle.cpp");
    const auto core = read("src/platform/linux/wayland_application_core_internal.hpp");
    const auto system_header = read("src/platform/linux/wayland_system_application_internal.hpp");
    const auto system = read("src/platform/linux/wayland_system_application.cpp");
    const auto dpi_header = read("src/platform/linux/wayland_dpi_awareness_internal.hpp");
    const auto dpi_source = read("src/platform/linux/wayland_dpi_awareness.cpp");
    const auto settings_header = read("src/platform/linux/wayland_portal_settings_internal.hpp");
    const auto settings = read("src/platform/linux/wayland_portal_settings.cpp");
    const auto settings_message = read("src/platform/linux/wayland_portal_settings_message.cpp");
    const auto monitor_header =
        read("src/platform/linux/wayland_portal_appearance_monitor_internal.hpp");
    const auto monitor = read("src/platform/linux/wayland_portal_appearance_monitor.cpp");
    const auto reader = read("src/platform/linux/wayland_portal_appearance_read.cpp");
    const auto signals = read("src/platform/linux/wayland_portal_appearance_signals.cpp");
    const auto module = read("build/xmake/capabilities/wayland_system_appearance_targets.lua");
    require(platform_neutral(appearance) && platform_neutral(dpi) &&
            platform_neutral(application),
            "public appearance/DPI headers must not expose D-Bus or Wayland");
    require(lines(source) <= 25 && has(source, "class WaylandEventSource") &&
            !has(source, "dbus"),
            "event source interface must stay protocol neutral");
    require(lines(loop) <= 110 && has(loop, "WaylandEventSource* source") &&
            has(loop, "source->dispatch()") && has(loop, "source->has_pending()") &&
            !has(loop, "dbus"),
            "event loop must poll the extra source without knowing D-Bus");
    require(lines(lifecycle) <= 25 && has(lifecycle, "appearance_event_source()") &&
            lines(core) <= 80 && has(core, "public WaylandSystemApplication"),
            "Wayland application must delegate to WaylandSystemApplication");
    require(lines(system_header) <= 40 && lines(system) <= 45 &&
            !has(system_header, "dbus") &&
            has(system_header, "std::unique_ptr<WaylandPortalAppearanceMonitor>") &&
            has(system, "wayland_dpi_awareness_state("),
            "system application must stay a thin owner of the monitor");
    require(lines(dpi_header) <= 20 && lines(dpi_source) <= 25 &&
            has(dpi_source, "per_monitor_aware_v2") &&
            has(dpi_source, "backend = \"wayland\""),
            "Wayland DPI awareness must live in its own module");
    require(lines(settings_header) <= 70 && lines(settings) <= 130 &&
            lines(settings_message) <= 60 &&
            has(settings_header, "org.freedesktop.appearance") &&
            has(settings, "\"color-scheme\"") && has(settings, "\"accent-color\"") &&
            has(settings, "\"contrast\"") && has(settings, "\"enable-animations\"") &&
            !has(settings, "dbus_connection_"),
            "portal value parsing must be pure and connection free");
    require(lines(monitor_header) <= 80 && lines(monitor) <= 110 &&
            lines(reader) <= 120 && lines(signals) <= 80 &&
            has(monitor, "dbus_bus_add_match(") && has(reader, "\"ReadOne\"") &&
            has(reader, "\"Read\"") && has(signals, "dbus_connection_dispatch(") &&
            has(signals, ".changed = changed"),
            "portal monitor must be split into connect, read, and signal modules");
    require(has(module, "is_plat(\"linux\")") &&
            has(module, "wayland_portal_settings_test") &&
            has(module, "wayland_portal_appearance_monitor_test") &&
            has(module, "wayland_dpi_awareness_test"),
            "Linux appearance targets must live in the guarded capability module");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Wayland system appearance structure failed: " << error.what() << '\n';
    return 1;
  }
}
