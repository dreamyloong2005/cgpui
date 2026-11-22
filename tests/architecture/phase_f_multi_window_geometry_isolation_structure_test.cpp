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
  const std::string activation = read_source("src/ui/runtime_window_activation.cpp");
  const std::string events = read_source("src/ui/runtime_event_windows.cpp");
  const std::string records = read_source("src/ui/runtime_window_records.cpp");
  const std::string root_frames = read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string child_frames = read_source("src/ui/runtime_window_frame_results.cpp");
  const std::string frames = root_frames + child_frames;
  const std::string resize = read_source("src/ui/runtime_renderer_resize_results.cpp");
  const std::string window_render = read_source("src/ui/runtime_window_rendering.cpp");
  const std::string frame_schedule = read_source("src/ui/runtime_frame_scheduling.cpp");
  const std::string frame_state = read_source("src/ui/runtime_frame_scheduling_internal.hpp");
  const std::string frame_declarations = read_source("include/cgpui/ui/detail/window_runtime_frame_scheduling_declarations.hpp");
  const std::string schedule_state = read_source("src/ui/runtime_state_scheduling_internal.hpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_geometry_test.cpp");
  const std::string manifests = read_source("tests/ui/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{&record, &activation, &events, &records,
      &root_frames, &child_frames, &resize, &window_render, &frame_schedule, &frame_state,
      &frame_declarations, &schedule_state,
      &scheduling, &run, &behavior, &manifests};
  for (const auto* value : required) if (value->empty()) return 1;
  if (!contains(record, "Size framebuffer_size{}") ||
      !contains(record, "Size viewport_size{}") ||
      !contains(record, "DpiScale scale{}") ||
      !contains(record, "bool redraw_scheduled = false")) return 2;
  if (!contains(activation, "record.framebuffer_size = window_state.framebuffer_size") ||
      !contains(activation, "record.viewport_size = to_logical_pixels(") ||
      !contains(events, "record->framebuffer_size = resized->size") ||
      !contains(events, "record->renderer->resize(resized->size, resized->scale)")) return 3;
  if (!contains(records, ".viewport_size = record.viewport_size") ||
      !contains(records, ".scale = record.scale") ||
      !contains(frames, ".viewport_size = record.viewport_size") ||
      !contains(frames, "record.viewport_size,") ||
      !contains(frames, "record.scale,")) return 4;
  if (contains(frames, "clear_invalidation();") ||
      contains(frames, "redraw_scheduled_ = false") ||
      contains(frames, "deferred_redraw_request_ = false") ||
      !contains(events, "record->redraw_scheduled = true") ||
      !contains(child_frames, "record.redraw_scheduled = false")) return 5;
  if (!contains(frame_declarations, "set_root_redraw_scheduled(bool scheduled)") ||
      !contains(schedule_state, "runtime_frame_scheduling_internal.hpp") ||
      !contains(frame_state, "bool rendering_frame_ = false") ||
      !contains(frame_state, "bool next_frame_redraw_requested_ = false") ||
      !contains(frame_schedule, "WindowRuntime::set_root_redraw_scheduled(") ||
      !contains(scheduling, "set_root_redraw_scheduled(true)") ||
      !contains(resize, "root_record->framebuffer_size = state_->windows.framebuffer_size_") ||
      !contains(run, "root_record->viewport_size = state_->windows.viewport_size_")) return 6;
  if (!contains(behavior, "root_pending_after") ||
      !contains(behavior, "child_redraw_consumed_before_render") ||
      !contains(behavior, "record->viewport_size.width == 400.0F") ||
      !contains(behavior, "renderers[0]->resize_count == 0")) return 7;
  if (line_count(record) > 65 || line_count(activation) > 70 ||
      line_count(events) > 125 || line_count(records) > 145 ||
      line_count(root_frames) > 173 || line_count(child_frames) > 120 || line_count(resize) > 30 ||
      line_count(window_render) > 20 || line_count(frame_schedule) > 50 ||
      line_count(frame_state) > 10 || line_count(frame_declarations) > 10 ||
      line_count(scheduling) > 135 ||
      line_count(run) > 145 || line_count(behavior) > 210) return 8;
  if (!contains(manifests, "\"window_runtime_multi_window_geometry_test.cpp\"") ||
      !contains(manifests, "\"phase_f_multi_window_geometry_isolation_structure_test.cpp\"")) return 9;
  return 0;
}
