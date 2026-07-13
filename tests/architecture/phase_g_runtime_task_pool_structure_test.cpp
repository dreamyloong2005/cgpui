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
  const std::string pool_header =
      read_source("src/ui/runtime_task_pool_internal.hpp");
  const std::string pool_source = read_source("src/ui/runtime_task_pool.cpp");
  const std::string priority_source =
      read_source("src/ui/runtime_task_priority.cpp");
  const std::string tasks_source = read_source("src/ui/runtime_tasks.cpp");
  const std::string shutdown_source = read_source("src/ui/runtime_shutdown.cpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string diagnostics_header =
      read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string diagnostics_source =
      read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  const std::string behavior =
      read_source("tests/async/runtime_task_pool_test.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
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
      &pool_header, &pool_source, &priority_source, &tasks_source, &shutdown_source,
      &runtime_internal, &diagnostics_header, &diagnostics_source, &behavior,
      &ui_structure, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(pool_header, "class WindowRuntime::RuntimeTaskPool") ||
      !contains(pool_header, "std::array<std::deque<Work>, 3> queues_") ||
      !contains(pool_header, "std::vector<std::jthread> workers_") ||
      !contains(pool_header, "std::size_t active_work_count_") ||
      !contains(pool_header, "std::size_t peak_active_work_count_") ||
      !contains(pool_header, "std::size_t completed_work_count_")) {
    return 2;
  }
  if (!contains(pool_source, "std::clamp<std::size_t>") ||
      !contains(pool_source, "RuntimeTaskPool::submit(") ||
      !contains(pool_source, "TaskPriority priority") ||
      !contains(pool_source, "RuntimeTaskPool::run_worker(") ||
      !contains(pool_source, "RuntimeTaskPool::shutdown()") ||
      !contains(pool_source, "queues_[runtime_task_priority_index(priority)]") ||
      !contains(pool_source, "workers.swap(workers_)")) {
    return 3;
  }
  if (!contains(runtime_internal, "std::unique_ptr<RuntimeTaskPool> task_pool_") ||
      contains(runtime_internal, "std::jthread worker;") ||
      !contains(priority_source, "task_pool_->submit(") ||
      contains(tasks_source, "std::jthread worker") ||
      !contains(priority_source, "(void)complete_task(id);")) {
    return 4;
  }
  const auto cancel = shutdown_source.find("task.cancellation_requested->store(true)");
  const auto shutdown = shutdown_source.find("task_pool_->shutdown();");
  if (cancel == std::string::npos || shutdown == std::string::npos ||
      cancel >= shutdown || contains(shutdown_source, "task.worker")) {
    return 5;
  }
  const char* diagnostics[]{
      "task_pool_worker_count", "task_pool_queued_work_count",
      "task_pool_active_work_count", "task_pool_peak_active_work_count",
      "task_pool_completed_work_count"};
  for (const char* field : diagnostics) {
    if (!contains(diagnostics_header, field) ||
        !contains(diagnostics_source, field)) return 6;
  }
  if (!contains(behavior, "task_count = worker_count * 2 + 1") ||
      !contains(behavior, "task_pool_queued_work_count == task_count - worker_count") ||
      !contains(behavior, "view.peak_active_count.load() == worker_count") ||
      !contains(behavior, "runtime.drain_task_completions()") ||
      !contains(behavior, "std::ranges::all_of(tasks")) {
    return 7;
  }
  if (!contains(ui_structure, "src/ui/runtime_task_pool_internal.hpp") ||
      !contains(ui_structure, "src/ui/runtime_task_pool.cpp") ||
      !contains(xmake, "target(\"phase_g_runtime_task_pool_test\")") ||
      !contains(xmake, "target(\"phase_g_runtime_task_pool_structure_test\")")) {
    return 8;
  }
  if (line_count(pool_header) > 60 || line_count(pool_source) > 120 ||
      line_count(priority_source) > 120 ||
      line_count(tasks_source) > 160 || line_count(shutdown_source) > 70 ||
      line_count(runtime_internal) > 260 || line_count(diagnostics_header) > 140 ||
      line_count(diagnostics_source) > 150 || line_count(behavior) > 150) {
    return 9;
  }

  constexpr const char* completion =
      "Phase G Step 635 replaces per-task background threads with a bounded "
      "reusable runtime task pool, preserves runtime-thread completion dispatch "
      "and cancellation tokens, and reports worker, queue, activity, peak, and "
      "completion diagnostics. Step 636 async task priority production behavior "
      "is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 670 examples and smoke test matrix coverage\"")) {
    return 11;
  }
  return 0;
}
