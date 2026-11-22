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
  const std::string protocol_header = read_source(
      "src/platform/linux/wayland_protocol_fractional_scale_internal.hpp");
  const std::string fractional_interfaces = read_source(
      "src/platform/linux/wayland_protocol_fractional_scale_interfaces.cpp");
  const std::string fractional_requests = read_source(
      "src/platform/linux/wayland_protocol_fractional_scale.cpp");
  const std::string viewporter_interfaces = read_source(
      "src/platform/linux/wayland_protocol_viewporter_interfaces.cpp");
  const std::string viewporter_requests = read_source(
      "src/platform/linux/wayland_protocol_viewporter.cpp");
  const std::string registry =
      read_source("src/platform/linux/wayland_registry.cpp");
  const std::string window =
      read_source("src/platform/linux/wayland_window.cpp");
  const std::string scale =
      read_source("src/platform/linux/wayland_window_scale.cpp");
  const std::string fractional = read_source(
      "src/platform/linux/wayland_window_fractional_scale.cpp");
  const std::string behavior =
      read_source("tests/platform/wayland_fractional_scale_test.cpp");
  const std::string compositor =
      read_source("tests/platform/wayland_test_compositor.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &protocol_header, &fractional_interfaces, &fractional_requests,
      &viewporter_interfaces, &viewporter_requests, &registry, &window, &scale,
      &fractional, &behavior, &compositor, &manifests};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(protocol_header, "wp_fractional_scale_v1_listener") ||
      !contains(fractional_interfaces, "preferred_scale") ||
      !contains(fractional_requests,
                "wp_fractional_scale_manager_v1_get_fractional_scale") ||
      !contains(viewporter_interfaces, "set_destination") ||
      !contains(viewporter_requests, "wp_viewport_set_destination")) return 2;
  if (!contains(registry, "wp_fractional_scale_manager_v1_interface.name") ||
      !contains(registry, "wp_viewporter_interface.name") ||
      !contains(window, "initialize_fractional_scale(") ||
      !contains(window, "wp_fractional_scale_v1_destroy(") ||
      !contains(window, "wp_viewport_destroy(")) return 3;
  if (!contains(scale, "std::ceil(scale)") ||
      !contains(scale, "preferred_fractional_scale_") ||
      !contains(fractional, "static_cast<float>(scale) / 120.0F") ||
      !contains(fractional, "wp_viewport_set_destination(")) return 4;
  if (!contains(behavior, "request_fractional_scale(150)") ||
      !contains(behavior, "state.scale.value, 1.25F") ||
      !contains(behavior, "last_surface_buffer_scale() == 2") ||
      !contains(behavior, "wait_until(scaled_surface)") ||
      !contains(compositor, "wp_fractional_scale_manager_v1_interface") ||
      !contains(compositor, "set_viewport_destination(") ||
      !contains(manifests, "\"wayland_fractional_scale_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_wayland_fractional_scale_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(protocol_header) > 50 || line_count(fractional) > 60 ||
      line_count(scale) > 80 || line_count(window) > 180 ||
      line_count(behavior) > 110) return 6;
  return 0;
}
