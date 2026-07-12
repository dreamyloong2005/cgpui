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
  const std::string public_header =
      read_source("include/cgpui/ui/task_priority.hpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string context_header =
      read_source("include/cgpui/ui/runtime_context.hpp");
  const std::string async_header =
      read_source("include/cgpui/ui/async_context.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/runtime.hpp");
  const std::string priority_internal =
      read_source("src/ui/runtime_task_priority_internal.hpp");
  const std::string priority_source =
      read_source("src/ui/runtime_task_priority.cpp");
  const std::string context_source =
      read_source("src/ui/runtime_context_task_priority.cpp");
  const std::string async_source =
      read_source("src/ui/async_context_task_priority.cpp");
  const std::string pool_header =
      read_source("src/ui/runtime_task_pool_internal.hpp");
  const std::string pool_source = read_source("src/ui/runtime_task_pool.cpp");
  const std::string tasks_source = read_source("src/ui/runtime_tasks.cpp");
  const std::string results_source =
      read_source("src/ui/runtime_task_results.cpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior =
      read_source("tests/async/runtime_task_priority_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_task_pool_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &public_header, &runtime_header, &context_header, &async_header,
      &aggregate, &priority_internal, &priority_source, &context_source,
      &async_source, &pool_header, &pool_source, &tasks_source, &results_source,
      &runtime_internal, &behavior, &header_cleanliness, &ui_structure,
      &previous, &xmake, &vocabulary, &core_parity, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "enum class TaskPriority") ||
      !contains(public_header, "low,") ||
      !contains(public_header, "normal,") ||
      !contains(public_header, "high,") ||
      contains(public_header, "class WindowRuntime")) return 2;
  const std::string* surfaces[]{&runtime_header, &context_header, &async_header};
  for (const auto* surface : surfaces) {
    if (!contains(*surface, "TaskPriority priority") ||
        !contains(*surface, "spawn_task(") ||
        !contains(*surface, "try_spawn_task(") ||
        !contains(*surface, "spawn_background_task(") ||
        !contains(*surface, "try_spawn_background_task(")) {
      return 3;
    }
  }
  if (!contains(aggregate, "cgpui/ui/task_priority.hpp") ||
      !contains(header_cleanliness, "cgpui/ui/task_priority.hpp") ||
      !contains(header_cleanliness, "TaskPriority::high") ||
      !contains(header_cleanliness, "TaskPriority::low")) return 4;
  if (!contains(priority_internal, "runtime_task_priorities_descending") ||
      !contains(priority_internal, "TaskPriority::high") ||
      !contains(priority_internal, "TaskPriority::normal") ||
      !contains(priority_internal, "TaskPriority::low") ||
      !contains(priority_internal, "runtime_task_priority_index")) return 5;
  if (!contains(priority_source, "TaskHandle WindowRuntime::spawn_task(") ||
      !contains(priority_source, "WindowRuntime::spawn_background_task(") ||
      !contains(priority_source, "WindowRuntime::try_spawn_task(") ||
      !contains(priority_source, "WindowRuntime::try_spawn_background_task(") ||
      !contains(priority_source, "task_pool_->submit(") ||
      !contains(context_source, "runtime.spawn_task(priority") ||
      !contains(async_source, "context_->spawn_task(priority")) return 6;
  if (!contains(pool_header, "std::array<std::deque<Work>, 3> queues_") ||
      !contains(pool_source, "runtime_task_priorities_descending") ||
      !contains(pool_source, "runtime_task_priority_index(priority)") ||
      !contains(runtime_internal, "TaskPriority priority = TaskPriority::normal") ||
      !contains(runtime_internal, "struct RuntimeTaskCompletion") ||
      !contains(tasks_source, "runtime_task_priorities_descending") ||
      !contains(tasks_source, "completion.priority != priority")) return 7;
  if (!contains(tasks_source, "TaskPriority::normal") ||
      !contains(results_source, "TaskPriority::normal") ||
      contains(results_source, "ErrorCode::invalid_argument")) return 8;
  if (!contains(behavior, "std::vector<int>{3, 2, 1}") ||
      !contains(behavior, "std::vector<int>{4, 5, 3, 1, 2}") ||
      !contains(behavior, "blocker_permits.store(1)") ||
      !contains(behavior, "TaskPriority::high") ||
      !contains(behavior, "TaskPriority::normal") ||
      !contains(behavior, "TaskPriority::low")) return 9;
  const char* source_paths[]{
      "src/ui/runtime_task_priority_internal.hpp",
      "src/ui/runtime_task_priority.cpp",
      "src/ui/runtime_context_task_priority.cpp",
      "src/ui/async_context_task_priority.cpp"};
  for (const char* path : source_paths) {
    if (!contains(ui_structure, path)) return 10;
  }
  if (!contains(previous, "std::array<std::deque<Work>, 3> queues_") ||
      !contains(xmake, "target(\"phase_g_runtime_task_priority_test\")") ||
      !contains(xmake, "target(\"phase_g_runtime_task_priority_structure_test\")")) {
    return 11;
  }
  if (line_count(public_header) > 30 || line_count(priority_internal) > 40 ||
      line_count(priority_source) > 120 || line_count(context_source) > 60 ||
      line_count(async_source) > 60 || line_count(pool_header) > 60 ||
      line_count(pool_source) > 120 || line_count(tasks_source) > 120 ||
      line_count(results_source) > 40 || line_count(runtime_internal) > 260 ||
      line_count(behavior) > 220) return 12;

  constexpr const char* completion =
      "Phase G Step 636 adds low, normal, and high task priorities across "
      "WindowRuntime, WindowRuntimeContext, and AsyncContextCapability, "
      "schedules queued background work and runtime-thread completions by "
      "priority with FIFO ordering within each priority, and preserves "
      "normal-priority compatibility for existing APIs. Step 637 structured "
      "task group production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 13;
  }
  if (!contains(vocabulary, "`TaskPriority::{low, normal, high}`") ||
      contains(vocabulary, "This freeze does not add task priorities") ||
      contains(core_parity, "task priority scheduling") ||
      !contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 647 animation cancellation "
          "production behavior\"")) {
    return 14;
  }
  return 0;
}
