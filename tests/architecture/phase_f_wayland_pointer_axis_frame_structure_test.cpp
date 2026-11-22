#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &state_header, &state, &callbacks, &core, &registry, &window_events,
      &behavior, &compositor, &manifests};
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
      !contains(manifests, "\"wayland_pointer_axis_frame_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_pointer_axis_frame_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(state_header) > 55 || line_count(state) > 100 ||
      line_count(callbacks) > 110 || line_count(behavior) > 130) {
    return 6;
  }
  return 0;
}
