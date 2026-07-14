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

} // namespace

int main() {
  const std::string pool = read_source(
      "tests/architecture/phase_g_runtime_task_pool_structure_test.cpp");
  const std::string priority = read_source(
      "tests/architecture/phase_g_runtime_task_priority_structure_test.cpp");
  const std::string groups = read_source(
      "tests/architecture/phase_g_runtime_task_group_structure_test.cpp");
  const std::string cancellation = read_source(
      "tests/architecture/phase_g_runtime_task_cancellation_propagation_structure_test.cpp");
  const std::string io = read_source(
      "tests/architecture/phase_g_runtime_async_io_hook_structure_test.cpp");
  const std::string timers = read_source(
      "tests/architecture/phase_g_runtime_async_timer_integration_structure_test.cpp");
  const std::string entities = read_source(
      "tests/architecture/phase_g_runtime_cross_thread_entity_access_structure_test.cpp");
  const std::string ui_source =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string platform_source =
      read_source("tests/architecture/platform_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
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
      &pool, &priority, &groups, &cancellation, &io, &timers, &entities,
      &ui_source, &platform_source, &header_cleanliness, &xmake, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(pool, "RuntimeTaskPool") ||
      !contains(priority, "runtime_task_priorities_descending") ||
      !contains(groups, "RuntimeTaskGroupStore") ||
      !contains(cancellation, "cancel_locked(runtime, *child") ||
      !contains(io, "RuntimeAsyncIoRegistry") ||
      !contains(timers, "nearest-deadline delayed wakeups") ||
      !contains(entities, "CrossThreadEntityQueueState")) return 2;

  const char* targets[]{
      "phase_g_runtime_task_pool_test",
      "phase_g_runtime_task_priority_test",
      "phase_g_runtime_task_group_test",
      "phase_g_runtime_task_cancellation_propagation_test",
      "phase_g_runtime_async_io_hook_test",
      "phase_g_runtime_async_timer_integration_test",
      "phase_g_runtime_cross_thread_entity_access_test",
      "phase_g_win32_timer_wakeup_test",
      "phase_g_wayland_timer_wakeup_test",
      "phase_g_runtime_task_pool_structure_test",
      "phase_g_runtime_task_priority_structure_test",
      "phase_g_runtime_task_group_structure_test",
      "phase_g_runtime_task_cancellation_propagation_structure_test",
      "phase_g_runtime_async_io_hook_structure_test",
      "phase_g_runtime_async_timer_integration_structure_test",
      "phase_g_runtime_cross_thread_entity_access_structure_test"};
  for (const char* target : targets) {
    if (!contains(xmake, (std::string{"target(\""} + target + "\")").c_str())) {
      return 3;
    }
  }

  const char* ui_sources[]{
      "runtime_task_pool.cpp", "runtime_task_priority.cpp",
      "runtime_task_group.cpp", "runtime_task_group_propagation.cpp",
      "runtime_async_io.cpp", "runtime_async_io_completion.cpp",
      "runtime_timer_integration.cpp", "runtime_cross_thread_entity_queue.cpp",
      "runtime_cross_thread_entity.cpp"};
  for (const char* source : ui_sources) {
    if (!contains(ui_source, source)) return 4;
  }
  if (!contains(platform_source, "win32_timer_wakeup.cpp") ||
      !contains(platform_source, "wayland_timer_wakeup.cpp") ||
      !contains(header_cleanliness, "CrossThreadEntity<TestModel>") ||
      !contains(xmake, "target(\"phase_g_async_runtime_closeout_test\")") ||
      !contains(roadmap, "- [x] Steps 635-642: Complete async runtime")) {
    return 5;
  }

  constexpr const char* completion =
      "Phase G Step 642 audits and closes the async runtime production band, "
      "freezing bounded priority scheduling, structured cancellation, async "
      "I/O, platform timers, cross-thread entity access, diagnostics, shutdown "
      "behavior, and modular source evidence. Step 643 animation transition "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 6;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 676 candidate-ledger closeout audit\"")) {
    return 7;
  }
  return 0;
}
