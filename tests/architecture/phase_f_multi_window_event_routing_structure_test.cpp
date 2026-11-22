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
  const std::string state = read_source("src/ui/runtime_window_event_state.cpp");
  const std::string state_header = read_source("include/cgpui/ui/detail/window_runtime_window_event_declarations.hpp");
  const std::string private_header = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string events = read_source("src/ui/runtime_event_windows.cpp");
  const std::string lifecycle = read_source("src/ui/runtime_event_lifecycle.cpp");
  const std::string dispatch = read_source("src/ui/runtime_event_dispatch.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string activation = read_source("src/ui/runtime_window_activation.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_event_routing_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&record, &state, &state_header, &private_header,
      &events, &lifecycle, &dispatch, &records, &activation, &run, &behavior,
      &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(record, "std::optional<EventRoute> event_route") ||
      !contains(record, "EventResult last_event_result{}") ||
      !contains(record, "std::optional<EventDispatchRecord> last_event_dispatch")) return 2;
  if (!contains(state, "WindowRuntime::begin_event_route_for_record(") ||
      !contains(state, "WindowRuntime::finish_event_dispatch_for_record(") ||
      !contains(state, "WindowRuntime::sync_root_event_record(") ||
      !contains(state, "WindowRuntime::reset_event_state_for_record(")) return 3;
  if (!contains(events, "begin_event_route_for_record(record, event)") ||
      !contains(events, "finish_event_dispatch_for_record(record, result)") ||
      contains(events, "current_event_route_ = EventRouter::route_to_root")) return 4;
  if (!contains(lifecycle, "finish_event_dispatch_for_record(record") ||
      !contains(dispatch, "sync_root_event_record()") ||
      !contains(records, ".event_route = record.event_route") ||
      !contains(records, ".last_event_dispatch = record.last_event_dispatch")) return 5;
  if (!contains(activation, "reset_event_state_for_record(record)") ||
      !contains(run, "reset_event_state_for_record(*root_record)") ||
      !contains(behavior, "root_record->last_event_dispatch->sequence == 1") ||
      !contains(behavior, "child_record->last_event_dispatch->sequence == 2")) return 6;
  if (!contains(ui_structure, "runtime_window_event_state_source") ||
      !contains(private_header, "window_runtime_window_event_declarations.hpp")) return 7;
  if (line_count(record) > 65 || line_count(state) > 60 ||
      line_count(state_header) > 20 || line_count(private_header) > 260 ||
      line_count(events) > 100 || line_count(lifecycle) > 60 ||
      line_count(dispatch) > 65 || line_count(records) > 145 ||
      line_count(activation) > 70 || line_count(run) > 141 ||
      line_count(behavior) > 210) return 8;
  if (!contains(manifests, "\"window_runtime_multi_window_event_routing_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_event_routing_structure_test.cpp\"")) return 9;
  return 0;
}
