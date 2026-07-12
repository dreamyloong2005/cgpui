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
  const std::string configure = read_source(
      "src/platform/linux/wayland_window_configure.cpp");
  const std::string configure_internal = read_source(
      "src/platform/linux/wayland_window_configure_internal.hpp");
  const std::string window_internal = read_source(
      "src/platform/linux/wayland_window_internal.hpp");
  const std::string helper = read_source(
      "tests/platform/wayland_test_configure_state.hpp");
  const std::string compositor_header = read_source(
      "tests/platform/wayland_test_compositor.hpp");
  const std::string compositor = read_source(
      "tests/platform/wayland_test_compositor.cpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_configure_lifecycle_test.cpp");
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
      &configure, &configure_internal, &window_internal, &helper,
      &compositor_header, &compositor, &behavior, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* source : required) if (source->empty()) return 1;
  if (!contains(configure, "bool WaylandWindow::acknowledge_configure(") ||
      !contains(configure, "pending_configure_.pending_size = false;") ||
      !contains(configure, "const bool resized =") ||
      !contains(configure, "if (was_configured && resized)") ||
      !contains(configure_internal,
                "[[nodiscard]] bool acknowledge_configure(")) return 2;
  if (contains(configure, "if (configured_)") ||
      contains(window_internal, "resize_pending_surface_configure_")) return 3;
  if (!contains(helper, "class WaylandTestConfigureState") ||
      !contains(helper, "take_toplevel_pending()") ||
      !contains(helper, "request_surface()") ||
      !contains(helper, "wait_for_toplevel_sent_count(")) return 4;
  if (!contains(compositor_header, "request_toplevel_configure_state(") ||
      !contains(compositor_header, "request_surface_configure()") ||
      !contains(compositor, "configure.take_toplevel_pending()") ||
      !contains(compositor, "configure.take_surface_pending()")) return 5;
  if (!contains(behavior,
                "request_toplevel_configure_state(640, 480") ||
      !contains(behavior,
                "request_toplevel_configure_state(0, 0") ||
      !contains(behavior, "wait_for_toplevel_configure_sent_count(4)") ||
      !contains(behavior, "resize_count.load() != 2") ||
      !contains(xmake, "target(\"wayland_configure_lifecycle_test\")") ||
      !contains(xmake,
                "target(\"phase_f_wayland_configure_lifecycle_structure_test\")")) {
    return 6;
  }
  if (line_count(configure) > 100 || line_count(configure_internal) > 25 ||
      line_count(helper) > 140 || line_count(behavior) > 120 ||
      line_count(compositor) > 3700 || line_count(window_internal) > 120) {
    return 7;
  }
  constexpr const char* completion =
      "Phase F Step 560 commits Wayland toplevel size and state only at "
      "surface configure acknowledgement, discards superseded pending sizes, "
      "and suppresses duplicate resize events. Step 561 Wayland cursor theme "
      "loading production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const std::string* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_step_560_remaining_gap\": \"Step 561 Wayland")) {
    return 9;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 612 WSL full-debug verification")) {
    return 10;
  }
  return 0;
}
