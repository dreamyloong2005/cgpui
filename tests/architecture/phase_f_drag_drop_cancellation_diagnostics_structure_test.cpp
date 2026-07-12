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
  const std::string public_header =
      read_source("include/cgpui/platform/platform_diagnostics.hpp");
  const std::string runtime = read_source("src/ui/runtime_event_input.cpp");
  const std::string behavior = read_source(
      "tests/ui/window_runtime_drag_drop_cancellation_diagnostics_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_f_clipboard_ownership_diagnostics_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&public_header, &runtime, &behavior, &previous,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "bool cancelled = false;")) return 2;
  if (!contains(runtime,
                "const bool cancelled = std::holds_alternative<DragExited>(event)") ||
      !contains(runtime, ".succeeded = !cancelled") ||
      !contains(runtime, ".cancelled = cancelled")) return 3;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "97 + index") ||
      !contains(behavior, "event.cancelled != cancellation") ||
      !contains(behavior, "event.succeeded == cancellation") ||
      !contains(behavior, "event.value_count != 1") ||
      !contains(previous, "Phase F Step 604")) return 4;
  if (line_count(public_header) > 45 || line_count(runtime) > 170 ||
      line_count(behavior) > 75) return 5;
  if (!contains(xmake,
                "target(\"window_runtime_drag_drop_cancellation_diagnostics_test\")") ||
      !contains(xmake,
                "target(\"phase_f_drag_drop_cancellation_diagnostics_structure_test\")")) return 6;
  constexpr const char* completion =
      "Phase F Step 605 marks drag exits as cancelled runtime diagnostics, preserves successful enter/drop reporting, and verifies 64 cancellation cycles retain a bounded 32-event sequence. Step 606 IME diagnostics and stress production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 7;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 655 asset cache key production behavior\"")) return 8;
  return 0;
}
