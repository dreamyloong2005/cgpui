#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
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
  const std::string event_header =
      read_source("include/cgpui/core/event_platform.hpp");
  const std::string diagnostic_header =
      read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string helper_header =
      read_source("src/ui/runtime_window_scale_diagnostics_internal.hpp");
  const std::string helper =
      read_source("src/ui/runtime_window_scale_diagnostics.cpp");
  const std::string root = read_source("src/ui/runtime_rendering.cpp");
  const std::string child = read_source("src/ui/runtime_event_windows.cpp");
  const std::string behavior =
      read_source("tests/ui/window_runtime_scale_diagnostics_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_f_ime_diagnostics_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&event_header, &diagnostic_header,
      &helper_header, &helper, &root, &child, &behavior, &previous, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(event_header, "window_resized") ||
      !contains(diagnostic_header, "window_scale") ||
      !contains(diagnostic_header, "Size framebuffer_size") ||
      !contains(diagnostic_header, "DpiScale scale")) return 2;
  if (!contains(helper_header, "window_scale_diagnostic(") ||
      !contains(helper, ".kind = PlatformDiagnosticKind::window_scale") ||
      !contains(helper, ".event_kind = EventKind::window_resized") ||
      !contains(helper, ".operation = \"resize-scale\"") ||
      !contains(helper, ".framebuffer_size = event.size") ||
      !contains(helper, ".scale = event.scale")) return 3;
  if (!contains(root, "window_scale_diagnostic(event, result.has_value())") ||
      !contains(child,
                "window_scale_diagnostic(*resized, resize_result.has_value())")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "source_index = static_cast<int>(index) + 32") ||
      !contains(behavior, "event.framebuffer_size.width") ||
      !contains(behavior, "event.scale.value") ||
      !contains(previous, "Phase F Step 606")) return 5;
  if (line_count(event_header) > 75 || line_count(diagnostic_header) > 50 ||
      line_count(helper_header) > 20 || line_count(helper) > 30 ||
      line_count(root) > 40 || line_count(child) > 105 ||
      line_count(behavior) > 65) return 6;
  if (!contains(xmake, "target(\"window_runtime_scale_diagnostics_test\")") ||
      !contains(xmake,
                "target(\"phase_f_scale_diagnostics_structure_test\")")) return 7;
  constexpr const char* completion =
      "Phase F Step 607 records root and child window resize-scale diagnostics with framebuffer and DPI snapshots, and verifies 64 scale changes retain the newest bounded 32-event sequence. Step 608 timer wakeup diagnostics and stress production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 667 Windows debug build and packaging coverage\"")) return 9;
  return 0;
}
