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
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string async = read_source("src/ui/test_app_async.cpp");
  const std::string behavior =
      read_source("tests/ui/test_app_async_control_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_test_app_timer_control_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &internal, &async, &behavior, &previous, &xmake, &vocabulary,
      &core, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "bool complete_task(TaskId id) const") ||
      !contains(header, "void drain_task_completions() const") ||
      !contains(internal, "bool complete_task(TaskId id)") ||
      !contains(internal, "void drain_task_completions()")) return 2;
  if (!contains(async, "TestAppState::complete_task(") ||
      !contains(async, "runtime.drain_task_completions()") ||
      !contains(async, "TestApp::drain_task_completions() const")) return 3;
  if (!contains(behavior, "TaskPriority::high") ||
      !contains(behavior, "normal_second") ||
      !contains(behavior, "TaskId{999999}") ||
      !contains(behavior, "app.run_until_parked()")) return 4;
  if (!contains(previous, "Phase I Step 759 X11/XCB platform boundary") ||
      !contains(xmake, "target(\"phase_g_test_app_async_control_test\")") ||
      !contains(xmake,
                "target(\"phase_g_test_app_async_control_structure_test\")")) {
    return 5;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(async) > 40 || line_count(behavior) > 105) return 6;

  constexpr const char* completion =
      "Phase G Step 662 adds deterministic TestApp async control for manual "
      "task completion, priority-ordered and FIFO draining, nested ready-task "
      "completion, invalid or repeated id rejection, and parked draining "
      "without a test-only executor. Step 663 GPUI-style rendering control "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(vocabulary, "`TestApp::complete_task(...)`") ||
      !contains(core, "Deterministic `TestApp` async control") ||
      !contains(ledger_json, "\"phase_g_step_662_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Phase I Step 759 X11/XCB platform boundary\"")) return 8;
  return 0;
}
