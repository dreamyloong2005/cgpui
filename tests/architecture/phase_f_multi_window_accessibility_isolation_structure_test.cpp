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
  const std::string record = read_source("include/cgpui/ui/runtime_window_options.hpp");
  const std::string root = read_source("src/ui/runtime_accessibility.cpp");
  const std::string window = read_source("src/ui/runtime_window_accessibility.cpp");
  const std::string window_header = read_source("include/cgpui/ui/detail/window_runtime_window_accessibility_declarations.hpp");
  const std::string static_accessibility = read_source("src/ui/runtime_static_accessibility.cpp");
  const std::string static_header = read_source("include/cgpui/ui/detail/window_runtime_static_rendering_declarations.hpp");
  const std::string root_frame = read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string child_frame = read_source("src/ui/runtime_window_frame_results.cpp");
  const std::string frame = root_frame + child_frame;
  const std::string activation = read_source("src/ui/runtime_window_activation.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string private_header = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_accessibility_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&record, &root, &window, &window_header,
      &static_accessibility, &static_header, &root_frame, &child_frame, &activation, &records,
      &run, &private_header, &behavior, &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(record, "last_accessibility_update") ||
      !contains(root, "update_platform_accessibility_tree_for_record(") ||
      contains(root, "window_->update_accessibility_tree")) return 2;
  if (!contains(window, "WindowRuntime::update_platform_accessibility_tree_for_record(") ||
      !contains(window, "record.last_accessibility_update") ||
      !contains(window, "record.window->update_accessibility_tree(") ||
      !contains(window, "record.runtime_id == state_->windows.root_window_runtime_id_")) return 3;
  if (!contains(static_accessibility, "WindowRuntime::static_accessibility_snapshot_for(") ||
      !contains(frame, "static_accessibility_snapshot_for(") ||
      !contains(frame, "record.input.keyboard_focus_element_owner") ||
      !contains(frame, "update_platform_accessibility_tree_for_record(")) return 4;
  if (!contains(activation, "reset_accessibility_state_for_record(record)") ||
      !contains(records, "reset_accessibility_state_for_record(record)") ||
      !contains(run, "reset_accessibility_state_for_record(*root_record)")) return 5;
  if (!contains(behavior, "root_history") ||
      !contains(behavior, "Child updated") ||
      !contains(behavior, "PlatformAccessibilityLiveUpdateKind::text_changed")) return 6;
  if (!contains(private_header, "window_runtime_window_accessibility_declarations.hpp") ||
      !contains(ui_structure, "runtime_window_accessibility_source")) return 7;
  if (line_count(record) > 70 || line_count(root) > 70 ||
      line_count(window) > 60 || line_count(window_header) > 8 ||
      line_count(static_accessibility) > 75 || line_count(static_header) > 25 ||
      line_count(root_frame) > 180 || line_count(child_frame) > 120 || line_count(private_header) > 260 ||
      line_count(behavior) > 260) return 8;
  if (!contains(manifests, "\"window_runtime_multi_window_accessibility_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_accessibility_isolation_structure_test.cpp\"")) return 9;
  return 0;
}
