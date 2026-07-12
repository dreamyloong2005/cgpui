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
  const char* guards[]{
      "tests/architecture/phase_f_window_churn_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_clipboard_ownership_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_drag_drop_cancellation_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_ime_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_scale_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_timer_wakeup_diagnostics_structure_test.cpp",
      "tests/architecture/phase_f_task_wakeup_diagnostics_structure_test.cpp",
  };
  for (const char* guard : guards) {
    if (read_source(guard).empty()) return 1;
  }
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
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 2;
  const char* behavior_targets[]{
      "window_runtime_multi_window_churn_test",
      "wayland_clipboard_ownership_test",
      "window_runtime_drag_drop_cancellation_diagnostics_test",
      "window_runtime_ime_diagnostics_test",
      "window_runtime_scale_diagnostics_test",
      "window_runtime_timer_wakeup_diagnostics_test",
      "window_runtime_task_wakeup_diagnostics_test",
  };
  for (const char* target : behavior_targets) {
    if (!contains(xmake, target)) return 3;
  }
  const char* structure_targets[]{
      "phase_f_window_churn_diagnostics_structure_test",
      "phase_f_clipboard_ownership_diagnostics_structure_test",
      "phase_f_drag_drop_cancellation_diagnostics_structure_test",
      "phase_f_ime_diagnostics_structure_test",
      "phase_f_scale_diagnostics_structure_test",
      "phase_f_timer_wakeup_diagnostics_structure_test",
      "phase_f_task_wakeup_diagnostics_structure_test",
      "phase_f_platform_diagnostics_stress_closeout_structure_test",
  };
  for (const char* target : structure_targets) {
    if (!contains(xmake, target)) return 4;
  }
  constexpr const char* completion =
      "Phase F Step 610 audits and closes the Steps 603-609 platform diagnostics and stress band, freezing window churn, clipboard ownership, drag cancellation, IME, scale, timer, and task evidence. Step 611 Windows full-debug verification is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 5;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 654 SVG asset decode boundary production behavior\"")) return 6;
  return 0;
}
