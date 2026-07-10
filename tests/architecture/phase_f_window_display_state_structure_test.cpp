#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
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
  const std::string platform_window =
      read_source("include/cgpui/platform/platform_window.hpp");
  const std::string lifecycle_default =
      read_source("src/platform/platform_window_lifecycle.cpp");
  const std::string win32_internal =
      read_source("src/platform/win32/win32_window_display_internal.hpp");
  const std::string win32_display =
      read_source("src/platform/win32/win32_window_display.cpp");
  const std::string win32_lifecycle =
      read_source("src/platform/win32/win32_window_lifecycle.cpp");
  const std::string win32_chrome =
      read_source("src/platform/win32/win32_window_chrome.cpp");
  const std::string wayland_internal =
      read_source("src/platform/linux/wayland_window_display_internal.hpp");
  const std::string wayland_display =
      read_source("src/platform/linux/wayland_window_display.cpp");
  const std::string wayland_registered =
      read_source("src/platform/linux/wayland_window_registered.cpp");
  const std::string protocol_internal =
      read_source("src/platform/linux/wayland_protocol_internal.hpp");
  const std::string protocol_toplevel =
      read_source("src/platform/linux/wayland_protocol_xdg_toplevel.cpp");
  const std::string protocol_display = read_source(
      "src/platform/linux/wayland_protocol_xdg_toplevel_display.cpp");
  const std::string protocol_interfaces =
      read_source("src/platform/linux/wayland_protocol_xdg_interfaces.cpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string win32_test =
      read_source("tests/platform/win32_window_display_state_test.cpp");
  const std::string wayland_test =
      read_source("tests/platform/wayland_window_display_state_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  if (platform_window.empty() || lifecycle_default.empty() ||
      win32_internal.empty() || win32_display.empty() ||
      win32_lifecycle.empty() || win32_chrome.empty() ||
      wayland_internal.empty() || wayland_display.empty() ||
      wayland_registered.empty() || protocol_internal.empty() ||
      protocol_toplevel.empty() || protocol_display.empty() ||
      protocol_interfaces.empty() ||
      compositor.empty() || win32_test.empty() || wayland_test.empty() ||
      xmake.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty()) {
    return 1;
  }
  if (!contains(platform_window, "request_display_state(") ||
      !contains(lifecycle_default, "PlatformWindow::request_display_state(") ||
      !contains(lifecycle_default, "return false;")) {
    return 2;
  }
  if (!contains(win32_internal, "bool fullscreen = false;") ||
      !contains(win32_internal, "RECT windowed_rect{};") ||
      !contains(win32_display, "Win32Window::request_display_state(") ||
      !contains(win32_display, "WS_OVERLAPPEDWINDOW") ||
      !contains(win32_display, "MONITOR_DEFAULTTONEAREST") ||
      !contains(win32_display, "SW_MINIMIZE") ||
      !contains(win32_display, "SW_MAXIMIZE") ||
      !contains(win32_display, "SW_RESTORE") ||
      !contains(win32_lifecycle, "display_command_state_.fullscreen") ||
      !contains(win32_chrome, "display_command_state_.windowed_style")) {
    return 3;
  }
  if (!contains(wayland_internal, "request_display_state(") ||
      !contains(wayland_display, "WaylandWindow::request_display_state(") ||
      !contains(wayland_display, "xdg_toplevel_set_minimized(") ||
      !contains(wayland_display, "xdg_toplevel_set_maximized(") ||
      !contains(wayland_display, "xdg_toplevel_set_fullscreen(") ||
      !contains(wayland_display, "wl_surface_commit(surface_)") ||
      !contains(wayland_registered,
                "RegisteredWaylandWindow::request_display_state(")) {
    return 4;
  }
  if (!contains(protocol_internal, "xdg_toplevel_set_maximized(") ||
      !contains(protocol_internal, "xdg_toplevel_unset_fullscreen(") ||
      !contains(protocol_display, "xdg_toplevel_request(toplevel, 13)") ||
      !contains(protocol_interfaces, "{\"set_maximized\", \"\", nullptr}") ||
      !contains(protocol_interfaces, "xdg_toplevel_set_fullscreen_types") ||
      !contains(protocol_interfaces, "{\"set_minimized\", \"\", nullptr}")) {
    return 5;
  }
  if (!contains(compositor, "maximize_requested.store(true)") ||
      !contains(compositor, "unfullscreen_requested.store(true)") ||
      !contains(win32_test, "MONITOR_DEFAULTTONEAREST") ||
      !contains(wayland_test, "wait_for_minimize_requested()") ||
      !contains(wayland_test, "wait_for_unfullscreen_requested()")) {
    return 6;
  }
  if (!contains(xmake, "target(\"win32_window_display_state_test\")") ||
      !contains(xmake, "target(\"wayland_window_display_state_test\")") ||
      !contains(xmake,
                "target(\"phase_f_window_display_state_structure_test\")")) {
    return 7;
  }
  if (line_count(lifecycle_default) > 30 || line_count(win32_internal) > 15 ||
      line_count(win32_display) > 100 || line_count(win32_lifecycle) > 70 ||
      line_count(wayland_internal) > 10 || line_count(wayland_display) > 55 ||
      line_count(protocol_toplevel) > 60 || line_count(protocol_display) > 55 ||
      line_count(protocol_interfaces) > 110) {
    return 8;
  }
  constexpr const char* completion =
      "Phase F Step 543 adds real Win32 minimize/maximize/restore and "
      "reversible borderless fullscreen, plus Wayland xdg-toplevel display "
      "requests with compositor-confirmed lifecycle state. Step 544 window "
      "positioning production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 9;
    }
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 544 window positioning")) {
    return 10;
  }
  return 0;
}
