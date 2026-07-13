#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines;
}

} // namespace

int main() {
  const std::string state_header = read_source(
      "src/platform/linux/wayland_pointer_scroll_frame_internal.hpp");
  const std::string state =
      read_source("src/platform/linux/wayland_pointer_scroll_frame.cpp");
  const std::string callbacks = read_source(
      "src/platform/linux/wayland_application_pointer_scroll.cpp");
  const std::string core = read_source(
      "src/platform/linux/wayland_application_core_internal.hpp");
  const std::string registry =
      read_source("src/platform/linux/wayland_registry.cpp");
  const std::string window_events =
      read_source("src/platform/linux/wayland_window_input_events.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_pointer_axis_frame_test.cpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string source_guard =
      read_source("tests/architecture/wayland_window_source_test.cpp");
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
      &state_header, &state, &callbacks, &core, &registry, &window_events,
      &behavior, &compositor, &source_guard, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) {
    if (source->empty()) return 1;
  }
  if (!contains(state_header, "struct WaylandPointerScrollFrameState") ||
      !contains(state_header, "value120_x") ||
      !contains(state_header, "stopped_axes") ||
      !contains(state, "high_resolution_wheel(") ||
      !contains(state, "WL_POINTER_AXIS_SOURCE_FINGER") ||
      !contains(state, "wayland_pointer_scroll_consume(")) {
    return 2;
  }
  if (!contains(callbacks, "wayland_pointer_scroll_axis(") ||
      !contains(callbacks, "wayland_pointer_scroll_source(") ||
      !contains(callbacks, "wayland_pointer_scroll_stop(") ||
      !contains(callbacks, "wayland_pointer_scroll_discrete(") ||
      !contains(callbacks, "wayland_pointer_scroll_value120(") ||
      !contains(callbacks, "frame.precise")) {
    return 3;
  }
  if (!contains(core, "WaylandPointerScrollFrameState pointer_scroll_frame_") ||
      !contains(registry, "std::min<std::uint32_t>(version, 9)") ||
      !contains(window_events, ".precise = precise")) {
    return 4;
  }
  if (!contains(behavior, "WL_POINTER_AXIS_SOURCE_FINGER") ||
      !contains(behavior, ".value120_x = 30") ||
      !contains(behavior, ".stop_x = true") ||
      !contains(compositor, "wl_pointer_send_axis_value120(") ||
      !contains(compositor, "wl_pointer_send_axis_stop(") ||
      !contains(compositor, "wl_pointer_interface,\n      std::min<std::uint32_t>(wl_resource_get_version(resource), 9)") ||
      !contains(xmake, "target(\"wayland_pointer_axis_frame_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_pointer_axis_frame_structure_test\")")) {
    return 5;
  }
  if (line_count(state_header) > 55 || line_count(state) > 100 ||
      line_count(callbacks) > 110 || line_count(behavior) > 130) {
    return 6;
  }
  constexpr const char* completion =
      "Phase F Step 558 aggregates Wayland axis frames with source-aware "
      "precision, high-resolution value120 metadata, stop-only cleanup, and "
      "v9 pointer negotiation. Step 559 Wayland fractional scale production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_558_remaining_gap\": \"Step 559 Wayland")) {
    return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 669 Windows and Linux release build and packaging coverage")) {
    return 9;
  }
  return 0;
}
