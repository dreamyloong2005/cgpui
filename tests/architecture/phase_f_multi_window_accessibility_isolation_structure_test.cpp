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
  const std::string record = read_source("include/cgpui/ui/runtime_window_options.hpp");
  const std::string root = read_source("src/ui/runtime_accessibility.cpp");
  const std::string window = read_source("src/ui/runtime_window_accessibility.cpp");
  const std::string window_header = read_source("src/ui/runtime_window_accessibility_internal.hpp");
  const std::string static_accessibility = read_source("src/ui/runtime_static_accessibility.cpp");
  const std::string static_header = read_source("src/ui/runtime_static_rendering_internal.hpp");
  const std::string frame = read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string activation = read_source("src/ui/runtime_window_activation.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string private_header = read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_accessibility_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_multi_window_theme_isolation_structure_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&record, &root, &window, &window_header,
      &static_accessibility, &static_header, &frame, &activation, &records,
      &run, &private_header, &behavior, &previous, &ui_structure, &xmake,
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(record, "last_accessibility_update") ||
      !contains(root, "update_platform_accessibility_tree_for_record(") ||
      contains(root, "window_->update_accessibility_tree")) return 2;
  if (!contains(window, "WindowRuntime::update_platform_accessibility_tree_for_record(") ||
      !contains(window, "record.last_accessibility_update") ||
      !contains(window, "record.window->update_accessibility_tree(") ||
      !contains(window, "record.runtime_id == root_window_runtime_id_")) return 3;
  if (!contains(static_accessibility, "WindowRuntime::static_accessibility_snapshot_for(") ||
      !contains(frame, "static_accessibility_snapshot_for(") ||
      !contains(frame, "record.input.keyboard_focus_element_owner") ||
      !contains(frame, "update_platform_accessibility_tree_for_record(")) return 4;
  if (!contains(activation, "reset_accessibility_state_for_record(record)") ||
      !contains(records, "reset_accessibility_state_for_record(record)") ||
      !contains(run, "reset_accessibility_state_for_record(*root_record)")) return 5;
  if (!contains(behavior, "root_history") ||
      !contains(behavior, "Child updated") ||
      !contains(behavior, "PlatformAccessibilityLiveUpdateKind::text_changed") ||
      !contains(previous, "Phase F Step 599")) return 6;
  if (!contains(private_header, "runtime_window_accessibility_internal.hpp") ||
      !contains(ui_structure, "runtime_window_accessibility_source")) return 7;
  if (line_count(record) > 70 || line_count(root) > 70 ||
      line_count(window) > 60 || line_count(window_header) > 8 ||
      line_count(static_accessibility) > 75 || line_count(static_header) > 25 ||
      line_count(frame) > 180 || line_count(private_header) > 260 ||
      line_count(behavior) > 260) return 8;
  if (!contains(xmake, "target(\"window_runtime_multi_window_accessibility_test\")") ||
      !contains(xmake, "target(\"phase_f_multi_window_accessibility_isolation_structure_test\")")) return 9;
  constexpr const char* completion =
      "Phase F Step 600 submits accessibility trees and live updates per runtime window, derives child snapshots from their own static render trees and focus state, and preserves root accessibility history across child frames. Step 601 multi-window lifecycle integration and churn production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 10;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF examples and asset closeout\"")) return 11;
  return 0;
}
