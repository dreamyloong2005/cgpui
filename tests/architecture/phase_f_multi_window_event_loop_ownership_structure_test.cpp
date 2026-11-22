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
  const std::string state = read_source("src/ui/runtime_window_ownership_state_internal.hpp");
  const std::string internal = read_source("include/cgpui/ui/detail/window_runtime_declarations.hpp");
  const std::string window_state = read_source("src/ui/runtime_state_windows_internal.hpp");
  const std::string lifetime_declarations =
      read_source("include/cgpui/ui/detail/window_runtime_window_lifetime_declarations.hpp");
  const std::string ownership = read_source("src/ui/runtime_window_ownership.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string events = read_source("src/ui/runtime_event_windows.cpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_event_loop_ownership_test.cpp");
  const std::string ui_structure = read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&state, &internal, &window_state, &lifetime_declarations, &ownership, &records,
      &events, &scheduling, &run, &behavior, &ui_structure, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(state, "native_additional_windows_") ||
      !contains(state, "retired_native_windows_") ||
      !contains(window_state, "runtime_window_ownership_state_internal.hpp") ||
      !contains(internal, "window_runtime_window_lifetime_declarations.hpp") ||
      !contains(lifetime_declarations, "collect_retired_native_windows()")) return 2;
  if (!contains(records, "retired_native_windows_.push_back(std::move(*owned_window))") ||
      !contains(records, "native_additional_windows_.erase(owned_window)") ||
      !contains(records, "request_platform_wakeup()") ||
      contains(records, "native_additional_windows_.erase(\n        std::remove_if")) return 3;
  if (!contains(ownership, "collect_retired_native_windows()") ||
      !contains(ownership, "release_retired(state_->windows.retired_native_windows_)") ||
      !contains(scheduling, "collect_retired_native_windows();") ||
      !contains(run, "collect_retired_native_windows();\n  deactivate_native_additional_windows();")) return 4;
  const std::size_t inactive = events.find("record == nullptr || !record->active");
  const std::size_t menu = events.find("handle_native_menu_command_event");
  if (inactive == std::string::npos || menu == std::string::npos ||
      inactive > menu) return 5;
  if (!contains(behavior, "wrapper_destroyed_during_callback") ||
      !contains(behavior, "deferred_after_close") ||
      !contains(behavior, "WindowWakeupRequested") ||
      !contains(behavior, "application.wakeup_count == 1")) return 6;
  if (!contains(ui_structure, "runtime_window_ownership_state_internal.hpp") ||
      line_count(state) > 5 || line_count(ownership) > 30 ||
      line_count(records) > 150 || line_count(events) > 120 ||
      line_count(scheduling) > 140 || line_count(run) > 141 ||
      line_count(internal) > 260 || line_count(lifetime_declarations) > 12 ||
      line_count(behavior) > 150) return 7;
  if (!contains(manifests, "\"window_runtime_event_loop_ownership_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_event_loop_ownership_structure_test.cpp\"")) return 8;
  return 0;
}
