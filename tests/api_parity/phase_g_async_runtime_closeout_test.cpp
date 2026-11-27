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
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/async/mcpp.toml") +
      read_source("tests/platform/mcpp.toml");
  const std::string* required[]{
      &pool,
      &priority,
      &groups,
      &cancellation,
      &io,
      &timers,
      &entities,
      &ui_source,
      &platform_source,
      &header_cleanliness,
      &manifests};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(pool, "RuntimeTaskPool") ||
      !contains(priority, "runtime_task_priorities_descending") ||
      !contains(groups, "RuntimeTaskGroupStore") ||
      !contains(cancellation, "cancel_locked(runtime, *child") ||
      !contains(io, "RuntimeAsyncIoRegistry") ||
      !contains(timers, "schedule_next_timer_wakeup()") ||
      !contains(entities, "CrossThreadEntityQueueState")) return 2;

  const char* targets[]{
      "runtime_task_pool_test.cpp",
      "runtime_task_priority_test.cpp",
      "runtime_task_group_test.cpp",
      "runtime_task_cancellation_propagation_test.cpp",
      "runtime_async_io_hook_test.cpp",
      "runtime_async_timer_integration_test.cpp",
      "runtime_cross_thread_entity_access_test.cpp",
      "win32_timer_wakeup_test.cpp",
      "wayland_timer_wakeup_test.cpp",
      "phase_g_runtime_task_pool_structure_test.cpp",
      "phase_g_runtime_task_priority_structure_test.cpp",
      "phase_g_runtime_task_group_structure_test.cpp",
      "phase_g_runtime_task_cancellation_propagation_structure_test.cpp",
      "phase_g_runtime_async_io_hook_structure_test.cpp",
      "phase_g_runtime_async_timer_integration_structure_test.cpp",
      "phase_g_runtime_cross_thread_entity_access_structure_test.cpp"};
  for (const char* target : targets) {
    if (!contains(manifests, (std::string{"\""} + target + "\"").c_str())) {
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
      !contains(manifests, "\"phase_g_async_runtime_closeout_test.cpp\"")) {
    return 5;
  }

  return 0;
}
