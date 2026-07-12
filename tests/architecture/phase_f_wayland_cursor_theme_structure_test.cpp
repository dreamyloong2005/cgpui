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
  const std::string cursor_header = read_source(
      "src/platform/linux/wayland_cursor_theme_internal.hpp");
  const std::string cursor = read_source(
      "src/platform/linux/wayland_cursor_theme.cpp");
  const std::string application_cursor = read_source(
      "src/platform/linux/wayland_application_cursor.cpp");
  const std::string application = read_source(
      "src/platform/linux/wayland_application.cpp");
  const std::string core = read_source(
      "src/platform/linux/wayland_application_core_internal.hpp");
  const std::string registry = read_source(
      "src/platform/linux/wayland_registry.cpp");
  const std::string scale = read_source(
      "src/platform/linux/wayland_window_scale.cpp");
  const std::string window_internal = read_source(
      "src/platform/linux/wayland_window_internal.hpp");
  const std::string helper = read_source(
      "tests/platform/wayland_test_cursor_state.hpp");
  const std::string compositor = read_source(
      "tests/platform/wayland_test_compositor.cpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_cursor_theme_test.cpp");
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
      &cursor_header, &cursor, &application_cursor, &application, &core,
      &registry, &scale, &window_internal, &helper, &compositor, &behavior,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(cursor, "wl_cursor_theme_load(") ||
      !contains(cursor, "wl_cursor_theme_get_cursor(") ||
      !contains(cursor, "wl_cursor_image_get_buffer(") ||
      !contains(cursor, "wl_surface_attach(") ||
      !contains(cursor, "wl_surface_set_buffer_scale(")) return 2;
  if (!contains(registry, "wl_shm_interface.name") ||
      !contains(core, "WaylandCursorThemeResourcesPtr cursor_theme_") ||
      !contains(application, "create_wayland_cursor_theme(") ||
      !contains(application, "cursor_theme_.reset()") ||
      !contains(application_cursor, "apply_wayland_cursor_theme(")) return 3;
  if (!contains(scale, "if (scale_changed_) scale_changed_(*this);") ||
      !contains(window_internal, "WaylandWindowScaleCallback scale_changed_") ||
      !contains(application_cursor,
                "WaylandApplication::cursor_scale_changed(")) return 4;
  if (!contains(helper, "class WaylandTestCursorState") ||
      !contains(compositor, "wl_display_init_shm(display)") ||
      !contains(compositor, "cursor.record_buffer_attached()") ||
      !contains(behavior, "write_cursor(") ||
      !contains(behavior, "request_fractional_scale(150)") ||
      !contains(behavior, "cursor.buffer_scale != 2")) return 5;
  if (!contains(xmake, "add_syslinks(\"wayland-cursor\"") ||
      !contains(xmake, "target(\"wayland_cursor_theme_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_cursor_theme_structure_test\")")) {
    return 6;
  }
  if (line_count(cursor_header) > 50 || line_count(cursor) > 140 ||
      line_count(application_cursor) > 80 || line_count(core) > 80 ||
      line_count(scale) > 80 || line_count(window_internal) > 120 ||
      line_count(helper) > 80 || line_count(compositor) > 3700 ||
      line_count(behavior) > 160) return 7;
  constexpr const char* completion =
      "Phase F Step 561 loads real Wayland cursor themes through wl_shm and "
      "libwayland-cursor, applies scaled cursor surfaces with image buffers "
      "and hotspots, and reloads on window scale changes. Step 562 Wayland "
      "event-loop wakeup production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_561_remaining_gap\": \"Step 562 Wayland")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 643 animation transition production behavior")) {
    return 10;
  }
  return 0;
}
