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
      read_source("include/cgpui/ui/task_group.hpp");
  const std::string ids_header =
      read_source("include/cgpui/ui/runtime_ids.hpp");
  const std::string runtime_header =
      read_source("include/cgpui/ui/window_runtime.hpp");
  const std::string context_header =
      read_source("include/cgpui/ui/runtime_context.hpp");
  const std::string async_header =
      read_source("include/cgpui/ui/async_context.hpp");
  const std::string aggregate = read_source("include/cgpui/ui/runtime.hpp");
  const std::string group_internal =
      read_source("src/ui/runtime_task_group_internal.hpp");
  const std::string group_source =
      read_source("src/ui/runtime_task_group.cpp");
  const std::string propagation_source =
      read_source("src/ui/runtime_task_group_propagation.cpp");
  const std::string handle_source = read_source("src/ui/task_group.cpp");
  const std::string context_source =
      read_source("src/ui/runtime_context_task_group.cpp");
  const std::string async_source =
      read_source("src/ui/async_context_task_group.cpp");
  const std::string tasks_source = read_source("src/ui/runtime_tasks.cpp");
  const std::string runtime_internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string behavior =
      read_source("tests/async/runtime_task_group_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_task_priority_structure_test.cpp");
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
      &public_header, &ids_header, &runtime_header, &context_header,
      &async_header, &aggregate, &group_internal, &group_source,
      &propagation_source,
      &handle_source, &context_source, &async_source, &tasks_source,
      &runtime_internal, &behavior, &header_cleanliness, &ui_structure,
      &previous, &xmake, &vocabulary, &core_parity, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "class TaskGroup") ||
      !contains(public_header, "~TaskGroup();") ||
      !contains(public_header, "TaskGroup(const TaskGroup&) = delete") ||
      !contains(public_header, "TaskGroup(TaskGroup&& other) noexcept") ||
      !contains(public_header, "std::size_t active_task_count() const") ||
      !contains(public_header, "bool complete() const") ||
      !contains(public_header, "bool cancelled() const") ||
      !contains(public_header, "bool cancel()") ||
      contains(public_header, "RuntimeTaskGroupStore")) return 2;
  if (!contains(public_header, "TaskPriority priority") ||
      !contains(public_header, "spawn_task(") ||
      !contains(public_header, "try_spawn_task(") ||
      !contains(public_header, "spawn_background_task(") ||
      !contains(public_header, "try_spawn_background_task(")) return 3;
  if (!contains(ids_header, "struct TaskGroupId") ||
      !contains(aggregate, "cgpui/ui/task_group.hpp") ||
      !contains(header_cleanliness, "cgpui/ui/task_group.hpp") ||
      !contains(header_cleanliness, "create_task_group()")) return 4;
  const std::string* surfaces[]{&runtime_header, &context_header, &async_header};
  for (const auto* surface : surfaces) {
    if (!contains(*surface, "TaskGroup create_task_group()")) return 5;
  }
  if (!contains(group_internal, "class WindowRuntime::RuntimeTaskGroupStore") ||
      !contains(group_internal, "std::vector<TaskId> task_ids") ||
      !contains(group_internal, "bool cancelled = false") ||
      !contains(runtime_internal, "TaskGroupId group_id{}") ||
      !contains(runtime_internal, "task_group_store_")) return 6;
  if (!contains(group_source, "group->task_ids.push_back(id)") ||
      !contains(group_source, "task_pool_->submit(") ||
      !contains(group_source, ".group_id = group_id") ||
      !contains(group_source, "runtime.tasks_mutex_") ||
      !contains(propagation_source, "std::ranges::all_of")) return 7;
  if (!contains(propagation_source, "cancellation_requested->store(true)") ||
      !contains(propagation_source, "task->cancelled = true") ||
      !contains(propagation_source, "task->queued = false") ||
      !contains(propagation_source, "std::erase_if(runtime.task_completion_queue_") ||
      !contains(tasks_source, "task->completed || task->cancelled")) return 8;
  if (!contains(handle_source, "TaskGroup::~TaskGroup() { (void)cancel(); }") ||
      !contains(handle_source, "std::exchange(other.runtime_, nullptr)") ||
      !contains(handle_source, "TaskPriority::normal") ||
      !contains(context_source, "runtime.create_task_group()") ||
      !contains(async_source, "context_->create_task_group()")) return 9;
  if (!contains(behavior, "cancellation_observed_count") ||
      !contains(behavior, "view.completion_count == 0") ||
      !contains(behavior, "normal_completion_matches") ||
      !contains(behavior, "completed.complete()") ||
      !contains(behavior, "static_assert(std::movable<cgpui::TaskGroup>)")) {
    return 10;
  }
  const char* source_paths[]{
      "src/ui/runtime_task_group_internal.hpp",
      "src/ui/runtime_task_group.cpp",
      "src/ui/runtime_task_group_propagation.cpp",
      "src/ui/task_group.cpp",
      "src/ui/runtime_context_task_group.cpp",
      "src/ui/async_context_task_group.cpp"};
  for (const char* path : source_paths) {
    if (!contains(ui_structure, path)) return 11;
  }
  if (!contains(previous, "Step 637 structured") ||
      !contains(xmake, "target(\"phase_g_runtime_task_group_test\")") ||
      !contains(xmake,
                "target(\"phase_g_runtime_task_group_structure_test\")")) {
    return 12;
  }
  if (line_count(public_header) > 80 || line_count(group_internal) > 70 ||
      line_count(group_source) > 220 || line_count(propagation_source) > 220 ||
      line_count(handle_source) > 160 ||
      line_count(context_source) > 30 || line_count(async_source) > 30 ||
      line_count(runtime_internal) > 260 || line_count(behavior) > 200) {
    return 13;
  }

  constexpr const char* completion =
      "Phase G Step 637 adds move-only structured task groups with runtime-owned "
      "membership, normal and explicit-priority spawning, active and total task "
      "observability, explicit bulk cancellation, and destructor cancellation "
      "that signals background tokens and suppresses cancelled completions. "
      "Step 638 task cancellation propagation production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 14;
  }
  if (!contains(vocabulary, "`TaskGroup`") ||
      contains(vocabulary, "does not add structured task groups") ||
      contains(core_parity, "priorities, structured task groups, pooled") ||
      !contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 649 style interpolation "
          "production behavior\"")) {
    return 15;
  }
  return 0;
}
