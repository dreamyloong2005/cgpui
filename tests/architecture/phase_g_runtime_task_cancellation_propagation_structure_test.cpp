#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string group_record =
      read_source("src/ui/runtime_task_group_record_internal.hpp");
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
  const std::string manifests = read_source("tests/async/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core_parity = read_source("docs/gpui-core-api-parity.md");
  const std::string* required[]{
      &public_header, &group_internal, &group_record, &group_source, &propagation_source,
      &handle_source, &tasks_source, &behavior, &header_cleanliness,
      &ui_structure, &previous, &manifests, &vocabulary, &core_parity};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(public_header, "TaskGroup create_child_group() const") ||
      !contains(handle_source, "TaskGroup TaskGroup::create_child_group() const") ||
      !contains(handle_source, "task_group_store_->create_child")) return 2;
  if (!contains(group_internal, "using Record = detail::RuntimeTaskGroupRecord") ||
      !contains(group_record, "TaskGroupId parent_id{}") ||
      !contains(group_record, "std::vector<TaskGroupId> child_ids") ||
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
      !contains(propagation_source, "std::erase_if(runtime.state_->tasks.task_completion_queue_") ||
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
      !contains(manifests,
                "\"runtime_task_cancellation_propagation_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_g_runtime_task_cancellation_propagation_structure_test.cpp\"")) {
    return 8;
  }
  if (line_count(public_header) > 80 || line_count(group_internal) > 90 ||
      line_count(group_record) > 25 ||
      line_count(group_source) > 140 || line_count(propagation_source) > 180 ||
      line_count(handle_source) > 160 || line_count(behavior) > 180 ||
      line_count(previous) > 200) return 9;

  if (!contains(vocabulary, "`TaskGroup::create_child_group()`") ||
      contains(core_parity, "Cancellation propagation depth")) {
    return 11;
  }
  return 0;
}
