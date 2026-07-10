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
  const std::string lifecycle_header = read_source(
      "include/cgpui/platform/platform_window_lifecycle.hpp");
  const std::string platform_window =
      read_source("include/cgpui/platform/platform_window.hpp");
  const std::string lifecycle_default =
      read_source("src/platform/platform_window_lifecycle.cpp");
  const std::string win32_internal =
      read_source("src/platform/win32/win32_window_internal.hpp");
  const std::string win32_lifecycle =
      read_source("src/platform/win32/win32_window_lifecycle.cpp");
  const std::string win32_message_internal =
      read_source("src/platform/win32/win32_window_message_internal.hpp");
  const std::string win32_proc_lifecycle =
      read_source("src/platform/win32/win32_window_proc_lifecycle.cpp");
  const std::string win32_events =
      read_source("src/platform/win32/win32_window_events.cpp");
  const std::string wayland_internal =
      read_source("src/platform/linux/wayland_window_internal.hpp");
  const std::string wayland_lifecycle =
      read_source("src/platform/linux/wayland_window_lifecycle.cpp");
  const std::string wayland_events =
      read_source("src/platform/linux/wayland_window_events.cpp");
  const std::string wayland_registered_internal =
      read_source("src/platform/linux/wayland_registered_window_internal.hpp");
  const std::string wayland_registered =
      read_source("src/platform/linux/wayland_window_registered.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");

  if (lifecycle_header.empty() || platform_window.empty() ||
      lifecycle_default.empty() || win32_internal.empty() ||
      win32_lifecycle.empty() || win32_message_internal.empty() ||
      win32_proc_lifecycle.empty() || win32_events.empty() ||
      wayland_internal.empty() || wayland_lifecycle.empty() ||
      wayland_events.empty() || wayland_registered_internal.empty() ||
      wayland_registered.empty() || xmake.empty() || roadmap.empty() ||
      ledger_md.empty() || ledger_json.empty() || task_plan.empty() ||
      findings.empty()) {
    return 1;
  }

  if (!contains(lifecycle_header, "enum class PlatformWindowDisplayState") ||
      !contains(lifecycle_header, "struct PlatformWindowLifecycleState") ||
      !contains(lifecycle_header, "bool native_window_created = false;") ||
      !contains(lifecycle_header, "bool initial_configure_complete = false;") ||
      !contains(lifecycle_header, "bool close_requested = false;")) {
    return 2;
  }

  if (!contains(platform_window,
                "#include \"cgpui/platform/platform_window_lifecycle.hpp\"") ||
      !contains(platform_window,
                "virtual PlatformWindowLifecycleState lifecycle_state() const;")) {
    return 3;
  }

  if (!contains(lifecycle_default,
                "PlatformWindow::lifecycle_state() const") ||
      !contains(lifecycle_default, ".close_requested = state().close_requested")) {
    return 4;
  }

  if (!contains(win32_internal,
                "PlatformWindowLifecycleState lifecycle_state() const override;") ||
      !contains(win32_lifecycle, "Win32Window::lifecycle_state() const") ||
      !contains(win32_lifecycle, "IsWindow(") ||
      !contains(win32_lifecycle, "IsIconic(") ||
      !contains(win32_lifecycle, "IsZoomed(")) {
    return 5;
  }

  if (!contains(win32_message_internal,
                "virtual void activation_changed(bool active) = 0;") ||
      !contains(win32_proc_lifecycle, "case WM_ACTIVATE:") ||
      !contains(win32_events, "Win32Window::activation_changed(bool active)") ||
      !contains(win32_events, "active_ = active;") ||
      !contains(win32_events, "focused_ = focused;") ||
      !contains(win32_lifecycle, ".active = created && active_") ||
      !contains(win32_lifecycle, ".focused = created && focused_")) {
    return 6;
  }

  if (!contains(wayland_internal,
                "PlatformWindowLifecycleState lifecycle_state() const override;") ||
      !contains(wayland_lifecycle, "WaylandWindow::lifecycle_state() const") ||
      !contains(wayland_lifecycle, "pending_configure_.configured") ||
      !contains(wayland_lifecycle,
                "pending_configure_.current_toplevel_state")) {
    return 7;
  }
  if (!contains(wayland_events, "focused_ = focused;") ||
      !contains(wayland_lifecycle, ".focused = focused_")) {
    return 8;
  }

  if (!contains(
          wayland_registered_internal,
          "PlatformWindowLifecycleState lifecycle_state() const override;") ||
      !contains(
          wayland_registered,
          "RegisteredWaylandWindow::lifecycle_state() const") ||
      !contains(wayland_registered, "return window_->lifecycle_state();")) {
    return 9;
  }

  if (line_count(lifecycle_header) > 35 ||
      line_count(lifecycle_default) > 25 ||
      line_count(win32_lifecycle) > 70 ||
      line_count(win32_proc_lifecycle) > 110 ||
      line_count(wayland_lifecycle) > 55 ||
      line_count(wayland_events) > 35) {
    return 10;
  }

  if (!contains(xmake, "target(\"phase_f_window_lifecycle_state_test\")") ||
      !contains(xmake,
                "target(\"phase_f_platform_lifecycle_structure_test\")") ||
      !contains(xmake,
                "target(\"win32_window_lifecycle_state_test\")") ||
      !contains(xmake,
                "target(\"wayland_window_lifecycle_state_test\")") ||
      !contains(
          xmake,
          "target(\"win32_window_activation_focus_state_test\")") ||
      !contains(
          xmake,
          "target(\"wayland_window_activation_focus_state_test\")")) {
    return 11;
  }

  constexpr const char* completion =
      "Phase F Step 539 adds a public window lifecycle snapshot with "
      "compatible defaults and real Win32/Wayland native-created, "
      "initial-configure, close-requested, and display-state reporting, "
      "including registered Wayland wrapper forwarding. Step 540 activation "
      "and focus production behavior is next.";
  const std::string* documents[]{
      &roadmap,
      &ledger_md,
      &ledger_json,
      &task_plan,
      &findings,
  };
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 10;
    }
  }
  if (!contains(
          ledger_json,
          "\"phase_f_step_539_remaining_gap\": \"Step 540 activation and focus")) {
    return 14;
  }
  constexpr const char* activation_focus_completion =
      "Phase F Step 540 makes activation and focus state-before-event "
      "observable on Win32 through `WM_ACTIVATE`/`WM_SETFOCUS`/"
      "`WM_KILLFOCUS` and on Wayland through xdg activated configures plus "
      "keyboard enter/leave, with lifecycle snapshots matching callback "
      "state. Step 541 resize and scale-change production behavior is next.";
  for (const std::string* document : documents) {
    if (!contains(*document, activation_focus_completion)) {
      return 12;
    }
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 541 resize and scale-change")) {
    return 13;
  }
  return 0;
}
