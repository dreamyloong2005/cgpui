#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) {
    source.open(path);
  }
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) {
    lines += value == '\n' ? 1U : 0U;
  }
  return lines;
}

} // namespace

int main() {
  const std::string event_pointer =
      read_source("include/cgpui/core/event_pointer.hpp");
  const std::string event_platform =
      read_source("include/cgpui/core/event_platform.hpp");
  const std::string application_pointer =
      read_source("src/platform/linux/wayland_application_pointer.cpp");
  const std::string window_api =
      read_source("src/platform/linux/wayland_window_api.hpp");
  const std::string window_bridge =
      read_source("src/platform/linux/wayland_window_bridge.cpp");
  const std::string window_events =
      read_source("src/platform/linux/wayland_window_input_events.cpp");
  const std::string event_kind =
      read_source("src/ui/ui_event_kind_internal.hpp");
  const std::string event_pointer_internal =
      read_source("src/ui/ui_event_pointer_internal.hpp");
  const std::string runtime_input =
      read_source("src/ui/runtime_event_input.cpp");
  const std::string runtime_windows =
      read_source("src/ui/runtime_event_windows.cpp");
  const std::string wayland_behavior =
      read_source("tests/platform/wayland_pointer_motion_test.cpp");
  const std::string runtime_behavior =
      read_source("tests/ui/window_runtime_pointer_exit_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &event_pointer, &event_platform, &application_pointer, &window_api,
      &window_bridge, &window_events, &event_kind, &event_pointer_internal,
      &runtime_input, &runtime_windows, &wayland_behavior, &runtime_behavior,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) {
      return 1;
    }
  }
  if (!contains(event_pointer, "struct PointerExited") ||
      !contains(event_platform, "PointerExited,") ||
      !contains(event_platform, "pointer_exited,")) {
    return 2;
  }
  if (!contains(application_pointer, "handle_pointer_enter(") ||
      !contains(application_pointer, "wayland_window_pointer_moved(") ||
      !contains(application_pointer, "pointer_enter_serial_ = 0") ||
      !contains(application_pointer, "pointer_scroll_frame_ = {}") ||
      !contains(application_pointer, "wayland_window_pointer_exited(")) {
    return 3;
  }
  if (!contains(window_api, "wayland_window_pointer_exited(") ||
      !contains(window_bridge, "wayland_window_pointer_exited(") ||
      !contains(window_events, "WaylandWindow::pointer_exited(")) {
    return 4;
  }
  if (!contains(event_kind, "PointerExited") ||
      !contains(event_pointer_internal, "std::get_if<PointerExited>") ||
      !contains(runtime_input, "std::get_if<PointerExited>") ||
      !contains(runtime_input, "hovered_element_id_.reset()") ||
      !contains(runtime_input, "CursorShape::default_arrow") ||
      !contains(runtime_windows, "std::get_if<PointerExited>")) {
    return 5;
  }
  if (!contains(wayland_behavior, "request_pointer_enter(10, 20)") ||
      !contains(wayland_behavior, "request_pointer_leave()") ||
      !contains(runtime_behavior, "EventKind::pointer_exited") ||
      !contains(runtime_behavior, "exit_cleared_hover") ||
      !contains(xmake, "target(\"wayland_pointer_motion_test\")") ||
      !contains(xmake, "target(\"window_runtime_pointer_exit_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_pointer_motion_structure_test\")")) {
    return 6;
  }
  if (line_count(application_pointer) > 80 || line_count(window_events) > 70 ||
      line_count(event_kind) > 120 || line_count(event_pointer_internal) > 90 ||
      line_count(runtime_input) > 160 || line_count(runtime_windows) > 120 ||
      line_count(wayland_behavior) > 130 || line_count(runtime_behavior) > 100) {
    return 7;
  }
  constexpr const char* completion =
      "Phase F Step 557 publishes Wayland pointer enter coordinates "
      "immediately, delivers explicit leave with the last position, and "
      "clears runtime hover and cursor state. Step 558 Wayland pointer axis "
      "and frame production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) {
      return 8;
    }
  }
  if (!contains(ledger_json,
                "\"phase_f_step_557_remaining_gap\": \"Step 558 Wayland")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 596 multi-window redraw and resize isolation")) {
    return 10;
  }
  return 0;
}
