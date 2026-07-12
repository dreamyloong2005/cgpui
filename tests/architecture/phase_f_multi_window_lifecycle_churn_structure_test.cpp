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
  const std::string ownership = read_source("src/ui/runtime_window_ownership.cpp");
  const std::string reclamation = read_source("src/ui/runtime_window_reclamation.cpp");
  const std::string reclamation_header = read_source("src/ui/runtime_window_reclamation_internal.hpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string private_header = read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_churn_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_multi_window_accessibility_isolation_structure_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&ownership, &reclamation,
      &reclamation_header, &records, &private_header, &behavior, &previous,
      &ui_structure, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(ownership, "retired_native_windows_.clear()") ||
      !contains(ownership, "reclaim_closed_window_records()") ||
      contains(ownership, "app_opened_windows_")) return 2;
  if (!contains(reclamation, "WindowRuntime::reclaim_closed_window_records(") ||
      !contains(reclamation, "reclaimable_closed_record(") ||
      !contains(reclamation, "app_opened_windows_") ||
      !contains(reclamation, "window_runtime_records_") ||
      !contains(reclamation, "window_themes_.erase(")) return 3;
  if (!contains(records, "retired_native_windows_.push_back(") ||
      !contains(records, "request_platform_wakeup()") ||
      !contains(private_header, "runtime_window_reclamation_internal.hpp")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "closing_window.has_value()") ||
      !contains(behavior, "window_runtime_records().size() != 1") ||
      !contains(behavior, "window_theme(opened.runtime_id) != nullptr") ||
      !contains(previous, "Phase F Step 600")) return 5;
  if (!contains(ui_structure, "runtime_window_reclamation.cpp") ||
      !contains(ui_structure, "runtime_window_reclamation_internal.hpp")) return 6;
  if (line_count(ownership) > 30 || line_count(reclamation) > 50 ||
      line_count(reclamation_header) > 3 || line_count(records) > 150 ||
      line_count(private_header) > 260 || line_count(behavior) > 210) return 7;
  if (!contains(xmake, "target(\"window_runtime_multi_window_churn_test\")") ||
      !contains(xmake, "target(\"phase_f_multi_window_lifecycle_churn_structure_test\")")) return 8;
  constexpr const char* completion =
      "Phase F Step 601 reclaims closed child runtime records, opened-window entries, and per-window themes only after deferred native destruction, preserves close-callback observability, and keeps repeated child-window churn bounded. Step 602 multi-window event-loop closeout audit is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 9;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 657 async asset loading production behavior\"")) return 10;
  return 0;
}
