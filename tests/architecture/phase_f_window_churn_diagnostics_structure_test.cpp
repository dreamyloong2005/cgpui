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
  const std::string diagnostics = read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string snapshot = read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_churn_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_multi_window_event_loop_closeout_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&diagnostics, &snapshot, &behavior, &previous,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  const char* fields[]{"runtime_window_record_count",
      "active_runtime_window_count", "opened_window_count",
      "active_native_child_window_count", "retired_native_child_window_count"};
  for (const char* field : fields) {
    if (!contains(diagnostics, field) || !contains(snapshot, field) ||
        !contains(behavior, field)) return 2;
  }
  if (!contains(snapshot, "for (const WindowRuntimeRecord& record") ||
      !contains(snapshot, "native_additional_windows_.size()") ||
      !contains(snapshot, "retired_native_windows_.size()") ||
      contains(snapshot, "collect_retired_native_windows()") ||
      contains(snapshot, "reclaim_closed_window_records()")) return 3;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "{1, 1, 0, 0, 0}") ||
      !contains(behavior, "{2, 2, 1, 1, 0}") ||
      !contains(behavior, "{2, 1, 1, 0, 1}") ||
      !contains(previous, "Phase F Step 602")) return 4;
  if (line_count(snapshot) > 150 || line_count(behavior) > 210) return 5;
  if (!contains(xmake, "target(\"window_runtime_multi_window_churn_test\")") ||
      !contains(xmake, "target(\"phase_f_window_churn_diagnostics_structure_test\")")) return 6;
  constexpr const char* completion =
      "Phase F Step 603 exposes bounded window lifecycle diagnostics for runtime records, active windows, opened windows, active native children, and retired native children, and verifies 64 churn cycles return to the root-only baseline. Step 604 clipboard ownership diagnostics and stress production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF examples and asset closeout\"")) return 8;
  return 0;
}
