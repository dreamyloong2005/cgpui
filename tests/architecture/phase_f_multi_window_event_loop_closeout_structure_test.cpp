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
} // namespace

int main() {
  const std::string ownership = read_source("tests/architecture/phase_f_multi_window_event_loop_ownership_structure_test.cpp");
  const std::string geometry = read_source("tests/architecture/phase_f_multi_window_geometry_isolation_structure_test.cpp");
  const std::string input = read_source("tests/architecture/phase_f_multi_window_input_isolation_structure_test.cpp");
  const std::string routing = read_source("tests/architecture/phase_f_multi_window_event_routing_structure_test.cpp");
  const std::string theme = read_source("tests/architecture/phase_f_multi_window_theme_isolation_structure_test.cpp");
  const std::string accessibility = read_source("tests/architecture/phase_f_multi_window_accessibility_isolation_structure_test.cpp");
  const std::string churn = read_source("tests/architecture/phase_f_multi_window_lifecycle_churn_structure_test.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string frame = read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string event_state = read_source("src/ui/runtime_window_event_state.cpp");
  const std::string input_state = read_source("src/ui/runtime_window_input_state.cpp");
  const std::string scheduling = read_source("src/ui/runtime_theme_scheduling.cpp");
  const std::string window_accessibility = read_source("src/ui/runtime_window_accessibility.cpp");
  const std::string reclamation = read_source("src/ui/runtime_window_reclamation.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&ownership, &geometry, &input, &routing,
      &theme, &accessibility, &churn, &records, &frame, &event_state,
      &input_state, &scheduling, &window_accessibility, &reclamation, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(ownership, "Phase F Step 595") ||
      !contains(geometry, "Phase F Step 596") ||
      !contains(input, "Phase F Step 597") ||
      !contains(routing, "Phase F Step 598") ||
      !contains(theme, "Phase F Step 599") ||
      !contains(accessibility, "Phase F Step 600") ||
      !contains(churn, "Phase F Step 601")) return 2;
  if (!contains(records, "retired_native_windows_.push_back(") ||
      !contains(frame, "try_draw_frame_for_record(") ||
      !contains(event_state, "begin_event_route_for_record(") ||
      !contains(input_state, "update_input_state_for_record(") ||
      !contains(scheduling, "request_all_theme_renders(") ||
      !contains(window_accessibility, "update_platform_accessibility_tree_for_record(") ||
      !contains(reclamation, "reclaim_closed_window_records(")) return 3;
  const char* targets[]{"window_runtime_event_loop_ownership_test",
      "window_runtime_multi_window_geometry_test",
      "window_runtime_multi_window_input_test",
      "window_runtime_multi_window_event_routing_test",
      "window_runtime_multi_window_theme_test",
      "window_runtime_multi_window_accessibility_test",
      "window_runtime_multi_window_churn_test",
      "phase_f_multi_window_event_loop_closeout_structure_test"};
  for (const char* target : targets) if (!contains(xmake, target)) return 4;
  constexpr const char* completion =
      "Phase F Step 602 audits and closes the Steps 595-601 multi-window event-loop band, freezing deferred native ownership, per-window geometry/input/routing/theme/accessibility isolation, close-callback observability, and bounded child-window churn. Step 603 window churn diagnostics and stress production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 5;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 613 cross-platform test execution audit\"")) return 6;
  return 0;
}
