#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
  const std::string frames = read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string resize = read_source("src/ui/runtime_renderer_resize_results.cpp");
  const std::string window_render = read_source("src/ui/runtime_window_rendering.cpp");
  const std::string frame_schedule = read_source("src/ui/runtime_frame_scheduling.cpp");
  const std::string frame_state = read_source("src/ui/runtime_frame_scheduling_internal.hpp");
  const std::string scheduling = read_source("src/ui/runtime_scheduling.cpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string behavior = read_source("tests/ui/window_runtime_multi_window_geometry_test.cpp");
  const std::string previous = read_source("tests/architecture/phase_f_multi_window_event_loop_ownership_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source("docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&record, &activation, &events, &records,
      &frames, &resize, &window_render, &frame_schedule, &frame_state,
      &scheduling, &run, &behavior, &previous, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
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
      !contains(window_render, "record.redraw_scheduled = false")) return 5;
  if (!contains(frame_state, "set_root_redraw_scheduled(bool scheduled)") ||
      !contains(frame_schedule, "WindowRuntime::set_root_redraw_scheduled(") ||
      !contains(scheduling, "set_root_redraw_scheduled(true)") ||
      !contains(resize, "root_record->framebuffer_size = framebuffer_size_") ||
      !contains(run, "root_record->viewport_size = viewport_size_")) return 6;
  if (!contains(behavior, "root_pending_after") ||
      !contains(behavior, "child_scheduled_during_render") ||
      !contains(behavior, "record->viewport_size.width == 400.0F") ||
      !contains(behavior, "renderers[0]->resize_count == 0")) return 7;
  if (line_count(record) > 65 || line_count(activation) > 70 ||
      line_count(events) > 125 || line_count(records) > 145 ||
      line_count(frames) > 165 || line_count(resize) > 30 ||
      line_count(window_render) > 20 || line_count(frame_schedule) > 50 ||
      line_count(frame_state) > 10 || line_count(scheduling) > 135 ||
      line_count(run) > 145 || line_count(behavior) > 210) return 8;
  if (!contains(previous, "Phase F Step 595") ||
      !contains(xmake, "target(\"window_runtime_multi_window_geometry_test\")") ||
      !contains(xmake, "target(\"phase_f_multi_window_geometry_isolation_structure_test\")")) return 9;
  constexpr const char* completion =
      "Phase F Step 596 stores framebuffer, viewport, scale, and redraw state per runtime window, routes child resize/render through that record, and preserves pending root invalidation across child frames. Step 597 multi-window input and focus isolation production behavior is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 10;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 635 async task pool production depth")) return 11;
  return 0;
}
