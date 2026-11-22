#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = CGPUI_SOURCE_ROOT; root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char character : text) {
    if (character == '\n') {
      ++lines;
    }
  }
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string position_header = read_source(
      "include/cgpui/platform/platform_window_position.hpp");
  const std::string platform_window =
      read_source("include/cgpui/platform/platform_window.hpp");
  const std::string window_header =
      read_source("include/cgpui/core/window.hpp");
  const std::string event_header =
      read_source("include/cgpui/core/event_window.hpp");
  const std::string event_platform =
      read_source("include/cgpui/core/event_platform.hpp");
  const std::string position_default =
      read_source("src/platform/platform_window_position.cpp");
  const std::string win32_application =
      read_source("src/platform/win32/win32_application.cpp");
  const std::string win32_factory =
      read_source("src/platform/win32/win32_window_factory.cpp");
  const std::string win32_position =
      read_source("src/platform/win32/win32_window_position.cpp");
  const std::string win32_proc = read_source(
      "src/platform/win32/win32_window_proc_lifecycle.cpp");
  const std::string wayland_position_internal = read_source(
      "src/platform/linux/wayland_window_position_internal.hpp");
  const std::string wayland_position =
      read_source("src/platform/linux/wayland_window_position.cpp");
  const std::string wayland_registered = read_source(
      "src/platform/linux/wayland_window_registered.cpp");
  const std::string runtime_control =
      read_source("src/ui/runtime_event_control.cpp");
  const std::string runtime_windows =
      read_source("src/ui/runtime_event_windows.cpp");
  const std::string event_kind =
      read_source("src/ui/ui_event_kind_internal.hpp");
  const std::string win32_test =
      read_source("tests/platform/win32_window_position_test.cpp");
  const std::string wayland_test =
      read_source("tests/platform/wayland_window_position_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &position_header, &platform_window, &window_header, &event_header,
      &event_platform, &position_default, &win32_application, &win32_factory,
      &win32_position,
      &win32_proc, &wayland_position_internal, &wayland_position,
      &wayland_registered, &runtime_control, &runtime_windows, &event_kind,
      &win32_test, &wayland_test, &manifests};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(position_header, "struct PlatformWindowPositionState") ||
      !contains(position_header, "bool supported = false;") ||
      !contains(position_header, "std::optional<Point> position;") ||
      !contains(platform_window, "position_state() const") ||
      !contains(platform_window, "request_position(Point position)") ||
      !contains(window_header, "std::optional<Point> position;") ||
      !contains(event_header, "struct WindowMoved")) {
    return 2;
  }
  if (!contains(position_default, "PlatformWindow::position_state() const") ||
      !contains(position_default, "PlatformWindow::request_position(") ||
      !contains(position_default, "return false;")) {
    return 3;
  }
  if (!contains(win32_factory, "descriptor.position.has_value()") ||
      !contains(win32_position, "GetWindowRect(hwnd_, &rect)") ||
      !contains(win32_position, "SetWindowPos(") ||
      !contains(win32_position, "display_command_state_.fullscreen") ||
      !contains(win32_position, "IsIconic(hwnd_)") ||
      !contains(win32_position, "IsZoomed(hwnd_)") ||
      !contains(win32_position, "callback_(WindowMoved") ||
      !contains(win32_proc, "case WM_MOVE:") ||
      !contains(win32_proc, "window->position_changed();")) {
    return 4;
  }
  if (!contains(wayland_position_internal, "position_state() const override") ||
      !contains(wayland_position, "WaylandWindow::position_state() const") ||
      !contains(wayland_position, "WaylandWindow::request_position(") ||
      !contains(wayland_position, "return false;") ||
      !contains(wayland_registered,
                "RegisteredWaylandWindow::position_state() const") ||
      !contains(wayland_registered,
                "RegisteredWaylandWindow::request_position(")) {
    return 5;
  }
  if (!contains(event_platform, "WindowMoved,") ||
      !contains(event_platform, "window_moved,") ||
      !contains(runtime_control, "std::holds_alternative<WindowMoved>") ||
      !contains(runtime_windows, "std::holds_alternative<WindowMoved>") ||
      !contains(event_kind, "EventKind::window_moved")) {
    return 6;
  }
  if (!contains(win32_test, "request_position(requested_position)") ||
      !contains(win32_test, "PlatformWindowDisplayState::fullscreen") ||
      !contains(wayland_test, "state.supported") ||
      !contains(wayland_test, "request_result") ||
      !contains(manifests, "\"win32_window_position_test.cpp\"") ||
      !contains(manifests, "\"wayland_window_position_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_window_position_structure_test.cpp\"")) {
    return 7;
  }
  if (line_count(position_header) > 20 || line_count(position_default) > 20 ||
      line_count(win32_position) > 45 || line_count(wayland_position) > 20 ||
      line_count(wayland_position_internal) > 8 ||
      line_count(runtime_control) > 75 || line_count(runtime_windows) > 115 ||
      line_count(win32_proc) > 110) {
    return 8;
  }
  return 0;
}
