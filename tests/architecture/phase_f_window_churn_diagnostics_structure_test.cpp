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
  const std::string diagnostics = read_source("include/cgpui/ui/runtime_diagnostics.hpp");
  const std::string snapshot = read_source("src/ui/runtime_diagnostic_snapshot.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_churn_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&diagnostics, &snapshot, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  const char* fields[]{"runtime_window_record_count",
      "active_runtime_window_count", "opened_window_count",
      "active_native_child_window_count", "retired_native_child_window_count"};
  for (const char* field : fields) {
    if (!contains(diagnostics, field) || !contains(snapshot, field) ||
        !contains(behavior, field)) return 2;
  }
  if (!contains(snapshot, "for (const WindowRuntimeRecord& record") ||
      !contains(snapshot, "native_additional_windows_.size()") ||
      !contains(snapshot, "retired_native_windows_.size()") ||
      contains(snapshot, "collect_retired_native_windows()") ||
      contains(snapshot, "reclaim_closed_window_records()")) return 3;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "{1, 1, 0, 0, 0}") ||
      !contains(behavior, "{2, 2, 1, 1, 0}") ||
      !contains(behavior, "{2, 1, 1, 0, 1}")) return 4;
  if (line_count(snapshot) > 150 || line_count(behavior) > 210) return 5;
  if (!contains(manifests, "\"window_runtime_multi_window_churn_test.cpp\"") ||
      !contains(manifests, "\"phase_f_window_churn_diagnostics_structure_test.cpp\"")) return 6;
  return 0;
}
