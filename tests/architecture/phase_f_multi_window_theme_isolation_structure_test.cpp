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
  const std::string theme = read_source("src/ui/runtime_theme.cpp");
  const std::string scheduling = read_source("src/ui/runtime_theme_scheduling.cpp");
  const std::string scheduling_header = read_source("src/ui/runtime_theme_scheduling_internal.hpp");
  const std::string private_header = read_source("src/ui/window_runtime_internal.hpp");
  const std::string context = read_source("src/ui/runtime_context_platform.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_theme_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_multi_window_event_routing_structure_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&theme, &scheduling, &scheduling_header,
      &private_header, &context, &behavior, &previous, &ui_structure, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(theme, "request_all_theme_renders()") ||
      !contains(theme, "request_theme_render(runtime_id)") ||
      contains(theme, "request_render();")) return 2;
  if (!contains(scheduling, "WindowRuntime::request_theme_render(") ||
      !contains(scheduling, "WindowRuntime::request_all_theme_renders(") ||
      !contains(scheduling, "record->redraw_scheduled = true") ||
      !contains(scheduling, "record->window->request_redraw()")) return 3;
  if (!contains(context, "runtime.theme_color(window_runtime_id, id)") ||
      !contains(context, "runtime.theme_spacing(window_runtime_id, id)") ||
      !contains(context, "runtime.set_window_theme(window_runtime_id")) return 4;
  if (!contains(behavior, "app_redraws_all") ||
      !contains(behavior, "child_redraw_isolated") ||
      !contains(behavior, "root_redraw_isolated") ||
      !contains(behavior, "clear_redraw_isolated")) return 5;
  if (!contains(private_header, "runtime_theme_scheduling_internal.hpp") ||
      !contains(ui_structure, "runtime_theme_scheduling_source") ||
      !contains(previous, "Phase F Step 598")) return 6;
  if (line_count(theme) > 75 || line_count(scheduling) > 35 ||
      line_count(scheduling_header) > 5 || line_count(private_header) > 260 ||
      line_count(context) > 70 || line_count(behavior) > 210) return 7;
  if (!contains(xmake, "target(\"window_runtime_multi_window_theme_test\")") ||
      !contains(xmake, "target(\"phase_f_multi_window_theme_isolation_structure_test\")")) return 8;
  constexpr const char* completion =
      "Phase F Step 599 schedules app-theme redraws across all active windows, confines window-theme redraws to the target runtime record, and preserves per-window token fallback in child contexts. Step 600 per-window accessibility isolation production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 9;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 613 cross-platform test execution audit")) return 10;
  return 0;
}
