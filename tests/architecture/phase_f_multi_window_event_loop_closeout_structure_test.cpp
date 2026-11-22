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
} // namespace

int main() {
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string frame = read_source("src/ui/runtime_window_frame_results.cpp");
  const std::string event_state = read_source("src/ui/runtime_window_event_state.cpp");
  const std::string input_state = read_source("src/ui/runtime_window_input_state.cpp");
  const std::string scheduling = read_source("src/ui/runtime_theme_scheduling.cpp");
  const std::string window_accessibility = read_source("src/ui/runtime_window_accessibility.cpp");
  const std::string reclamation = read_source("src/ui/runtime_window_reclamation.cpp");
  const std::string manifests = read_source("mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/ui/mcpp.toml");
  const std::string* required[]{&records, &frame, &event_state,
      &input_state, &scheduling, &window_accessibility, &reclamation, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(records, "retired_native_windows_.push_back(") ||
      !contains(frame, "try_draw_frame_for_record(") ||
      !contains(event_state, "begin_event_route_for_record(") ||
      !contains(input_state, "update_input_state_for_record(") ||
      !contains(scheduling, "request_all_theme_renders(") ||
      !contains(window_accessibility, "update_platform_accessibility_tree_for_record(") ||
      !contains(reclamation, "reclaim_closed_window_records(")) return 3;
  const char* targets[]{"window_runtime_event_loop_ownership_test.cpp",
      "window_runtime_multi_window_geometry_test.cpp",
      "window_runtime_multi_window_input_test.cpp",
      "window_runtime_multi_window_event_routing_test.cpp",
      "window_runtime_multi_window_theme_test.cpp",
      "window_runtime_multi_window_accessibility_test.cpp",
      "window_runtime_multi_window_churn_test.cpp",
      "phase_f_multi_window_event_loop_closeout_structure_test.cpp"};
  for (const char* target : targets) if (!contains(manifests, target)) return 4;
  return 0;
}
