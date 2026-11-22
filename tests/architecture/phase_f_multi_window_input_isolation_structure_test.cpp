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
  const std::string state = read_source("src/ui/runtime_window_input_state.cpp");
  const std::string state_header = read_source("include/cgpui/ui/detail/window_runtime_window_input_declarations.hpp");
  const std::string private_header = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string context = read_source("src/ui/runtime_context_input.cpp");
  const std::string events = read_source("src/ui/runtime_event_windows.cpp");
  const std::string dispatch = read_source("src/ui/runtime_event_dispatch.cpp");
  const std::string lifecycle = read_source("src/ui/runtime_event_lifecycle.cpp");
  const std::string focus = read_source("src/ui/runtime_focus.cpp");
  const std::string capture = read_source("src/ui/runtime_pointer_capture.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_input_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&record, &state, &state_header, &private_header,
      &context, &events, &dispatch, &lifecycle, &focus, &capture, &records,
      &behavior, &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(record, "ViewInputState input{}") ||
      !contains(state, "WindowRuntime::update_input_state_for_record(") ||
      !contains(state, "WindowRuntime::sync_root_input_record(") ||
      !contains(private_header, "window_runtime_window_input_declarations.hpp")) return 2;
  if (!contains(context, "capture_pointer_for_window(window_runtime_id") ||
      !contains(context, "request_keyboard_focus_for_window(window_runtime_id") ||
      !contains(context, "release_keyboard_focus_for_window(window_runtime_id")) return 3;
  if (!contains(events, "update_input_state_for_record(record, event)") ||
      contains(events, "input_.pointer_position") ||
      !contains(records, ".input = record.input") ||
      !contains(focus, "sync_root_input_record()") ||
      !contains(capture, "sync_root_input_record()")) return 4;
  if (!contains(dispatch, "sync_root_input_record()") ||
      !contains(lifecycle, "sync_root_input_record()") ||
      !contains(behavior, "root_record->input") ||
      !contains(behavior, "application.child.pointer_captured")) return 5;
  if (!contains(ui_structure, "runtime_window_input_state_source") ||
      !contains(ui_structure, "line_count(runtime_window_input_state_source) > 140")) return 6;
  if (line_count(record) > 65 || line_count(state) > 140 ||
      line_count(state_header) > 30 || line_count(private_header) > 260 ||
      line_count(context) > 90 || line_count(events) > 110 ||
      line_count(focus) > 90 || line_count(capture) > 50 ||
      line_count(records) > 145 || line_count(behavior) > 210) return 7;
  if (!contains(manifests, "\"window_runtime_multi_window_input_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_input_isolation_structure_test.cpp\"")) return 8;
  return 0;
}
