#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string runtime_record_input =
      read_source("src/ui/runtime_window_input_state.cpp");
  const std::string wayland_behavior =
      read_source("tests/platform/wayland_pointer_motion_test.cpp");
  const std::string runtime_behavior =
      read_source("tests/ui/window_runtime_pointer_exit_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &event_pointer, &event_platform, &application_pointer, &window_api,
      &window_bridge, &window_events, &event_kind, &event_pointer_internal,
      &runtime_input, &runtime_windows, &runtime_record_input, &wayland_behavior,
      &runtime_behavior,
      &manifests};
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
      !contains(runtime_windows, "update_input_state_for_record(record, event)") ||
      !contains(runtime_record_input, "pointer_position_for(event)")) {
    return 5;
  }
  if (!contains(wayland_behavior, "request_pointer_enter(10, 20)") ||
      !contains(wayland_behavior, "request_pointer_leave()") ||
      !contains(runtime_behavior, "EventKind::pointer_exited") ||
      !contains(runtime_behavior, "exit_cleared_hover") ||
      !contains(manifests, "\"wayland_pointer_motion_test.cpp\"") ||
      !contains(manifests, "\"window_runtime_pointer_exit_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_pointer_motion_structure_test.cpp\"")) {
    return 6;
  }
  if (line_count(application_pointer) > 80 || line_count(window_events) > 70 ||
      line_count(event_kind) > 120 || line_count(event_pointer_internal) > 90 ||
      line_count(runtime_input) > 160 || line_count(runtime_windows) > 120 ||
      line_count(wayland_behavior) > 130 || line_count(runtime_behavior) > 100) {
    return 7;
  }
  return 0;
}
