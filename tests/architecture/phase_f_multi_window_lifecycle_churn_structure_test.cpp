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
  const std::string ownership = read_source("src/ui/runtime_window_ownership.cpp");
  const std::string lifetime = read_source("src/ui/runtime_window_lifetime.cpp");
  const std::string reclamation = read_source("src/ui/runtime_window_reclamation.cpp");
  const std::string reclamation_header = read_source("include/cgpui/ui/detail/window_runtime_window_reclamation_declarations.hpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string private_header = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_churn_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&ownership, &lifetime, &reclamation,
      &reclamation_header, &records, &private_header, &behavior, &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(ownership, "release_retired(state_->windows.retired_native_windows_)") ||
      !contains(lifetime, "auto windows = std::move(native_windows)") ||
      !contains(lifetime, "renderers.clear();\n    windows.clear();") ||
      ownership.find("release_retired(state_->windows.retired_native_windows_)") >
          ownership.find("reclaim_closed_window_records()") ||
      !contains(ownership, "reclaim_closed_window_records()") ||
      contains(ownership, "app_opened_windows_")) return 2;
  if (!contains(reclamation, "WindowRuntime::reclaim_closed_window_records(") ||
      !contains(reclamation, "reclaimable_closed_record(") ||
      !contains(reclamation, "app_opened_windows_") ||
      !contains(reclamation, "window_runtime_records_") ||
      !contains(reclamation, "window_themes_.erase(")) return 3;
  if (!contains(records, "retired_native_windows_.push_back(") ||
      !contains(records, "request_platform_wakeup()") ||
      !contains(private_header, "window_runtime_window_reclamation_declarations.hpp")) return 4;
  if (!contains(behavior, "index < 64") ||
      !contains(behavior, "closing_window.has_value()") ||
      !contains(behavior, "window_runtime_records().size() != 1") ||
      !contains(behavior, "window_theme(opened.runtime_id) != nullptr")) return 5;
  if (!contains(ui_structure, "runtime_window_reclamation.cpp") ||
      !contains(ui_structure, "window_runtime_window_reclamation_declarations.hpp")) return 6;
  if (line_count(ownership) > 30 || line_count(reclamation) > 50 ||
      line_count(reclamation_header) > 3 || line_count(records) > 150 ||
      line_count(private_header) > 260 || line_count(behavior) > 210) return 7;
  if (!contains(manifests, "\"window_runtime_multi_window_churn_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_lifecycle_churn_structure_test.cpp\"")) return 8;
  return 0;
}
