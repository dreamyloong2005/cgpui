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
  const std::string group_internal =
      read_source("src/ui/runtime_task_group_internal.hpp");
  const std::string group_source =
      read_source("src/ui/runtime_task_group.cpp");
  const std::string propagation_source =
      read_source("src/ui/runtime_task_group_propagation.cpp");
  const std::string handle_source = read_source("src/ui/task_group.cpp");
  const std::string tasks_source = read_source("src/ui/runtime_tasks.cpp");
  const std::string behavior = read_source(
      "tests/async/runtime_task_cancellation_propagation_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string ui_structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_runtime_task_group_structure_test.cpp");
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
      &public_header, &group_internal, &group_source, &propagation_source,
      &handle_source, &tasks_source, &behavior, &header_cleanliness,
      &ui_structure, &previous, &xmake, &vocabulary, &core_parity, &roadmap,
      &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "TaskGroup create_child_group() const") ||
      !contains(handle_source, "TaskGroup TaskGroup::create_child_group() const") ||
      !contains(handle_source, "task_group_store_->create_child")) return 2;
  if (!contains(group_internal, "TaskGroupId parent_id{}") ||
      !contains(group_internal, "std::vector<TaskGroupId> child_ids") ||
      !contains(group_internal, "task_count_locked(") ||
      !contains(group_internal, "active_task_count_locked(") ||
      !contains(group_internal, "complete_locked(") ||
      !contains(group_internal, "cancel_locked(")) return 3;
  if (!contains(propagation_source, "RuntimeTaskGroupStore::create_child(") ||
      !contains(propagation_source, "parent->child_ids.push_back(id)") ||
      !contains(propagation_source, ".parent_id = parent_id") ||
      !contains(propagation_source, "parent->cancelled")) return 4;
  if (!contains(propagation_source, "task_count_locked(runtime, *child)") ||
      !contains(propagation_source,
                "active_task_count_locked(runtime, *child)") ||
      !contains(propagation_source, "complete_locked(runtime, *child)")) {
    return 5;
  }
  if (!contains(propagation_source,
                "cancel_locked(runtime, *child, cancelled_ids)") ||
      !contains(propagation_source, "cancellation_requested->store(true)") ||
      !contains(propagation_source, "std::erase_if(runtime.task_completion_queue_") ||
      !contains(tasks_source, "task->completed || task->cancelled")) return 6;
  if (!contains(behavior, "parent.task_count() == 4") ||
      !contains(behavior, "child.task_count() == 2") ||
      !contains(behavior, "child_isolated") ||
      !contains(behavior, "!parent.cancelled() && !sibling.cancelled()") ||
      !contains(behavior, "rejected_child.id().value == 0") ||
      !contains(behavior, "ancestor_destructor_matches") ||
      !contains(behavior, "view.completion_count == 0")) return 7;
  if (!contains(header_cleanliness, "create_child_group()") ||
      !contains(ui_structure, "src/ui/runtime_task_group_propagation.cpp") ||
      !contains(previous, "runtime_task_group_propagation.cpp") ||
      !contains(xmake,
                "target(\"phase_g_runtime_task_cancellation_propagation_test\")") ||
      !contains(
          xmake,
          "target(\"phase_g_runtime_task_cancellation_propagation_structure_test\")")) {
    return 8;
  }
  if (line_count(public_header) > 80 || line_count(group_internal) > 90 ||
      line_count(group_source) > 140 || line_count(propagation_source) > 180 ||
      line_count(handle_source) > 160 || line_count(behavior) > 180 ||
      line_count(previous) > 200) return 9;

  constexpr const char* completion =
      "Phase G Step 638 adds parent/child task groups with recursive subtree "
      "observability, descendant cancellation and destructor propagation, "
      "child/sibling isolation, and cancelled-ancestor spawn rejection. Step "
      "639 async I/O hook production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(vocabulary, "`TaskGroup::create_child_group()`") ||
      contains(core_parity, "Cancellation propagation depth") ||
      !contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 670 examples and smoke test matrix coverage\"")) {
    return 11;
  }
  return 0;
}
