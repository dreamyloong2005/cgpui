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
      &protocol_header, &fractional_interfaces, &fractional_requests,
      &viewporter_interfaces, &viewporter_requests, &registry, &window, &scale,
      &fractional, &behavior, &compositor, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
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
      !contains(behavior, "last_surface_buffer_scale() != 2") ||
      !contains(compositor, "wp_fractional_scale_manager_v1_interface") ||
      !contains(compositor, "set_viewport_destination(") ||
      !contains(xmake, "target(\"wayland_fractional_scale_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_fractional_scale_structure_test\")")) {
    return 5;
  }
  if (line_count(protocol_header) > 50 || line_count(fractional) > 60 ||
      line_count(scale) > 80 || line_count(window) > 180 ||
      line_count(behavior) > 110) return 6;
  constexpr const char* completion =
      "Phase F Step 559 adds production Wayland fractional scaling with "
      "preferred 120-based scale, integer-ceiling buffer scale, viewporter "
      "logical destinations, and integer output fallback. Step 560 Wayland "
      "configure lifecycle production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_559_remaining_gap\": \"Step 560 Wayland")) {
    return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 667 Windows debug build and packaging coverage")) {
    return 9;
  }
  return 0;
}
