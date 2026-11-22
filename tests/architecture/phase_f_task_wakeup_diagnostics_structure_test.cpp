#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string tasks = read_source("src/ui/runtime_tasks.cpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string behavior = read_source(
      "tests/ui/window_runtime_task_wakeup_diagnostics_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&public_header, &tasks, &scheduling, &behavior,
      &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(public_header, "scheduling")) return 2;
  if (!contains(tasks, "task->completed = true") ||
      !contains(tasks, ".kind = PlatformDiagnosticKind::scheduling") ||
      !contains(tasks, ".operation = \"task-completed\"") ||
      !contains(tasks,
                ".value_count = static_cast<std::size_t>(id.value)") ||
      !contains(scheduling, "drain_task_completions();")) return 3;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "request_wakeup_count != 64") ||
      !contains(behavior, "app.dispatch_wakeup()") ||
      !contains(behavior, "completed_task_count != 64") ||
      !contains(behavior, "platform_diagnostics.size() != 32") ||
      !contains(behavior, "index + 33")) return 4;
  if (line_count(public_header) > 50 || line_count(tasks) > 165 ||
      line_count(scheduling) > 140 || line_count(behavior) > 70) return 5;
  if (!contains(manifests,
                "\"window_runtime_task_wakeup_diagnostics_test.cpp\"") ||
      !contains(manifests,
                "\"phase_f_task_wakeup_diagnostics_structure_test.cpp\"")) return 6;
  return 0;
}
