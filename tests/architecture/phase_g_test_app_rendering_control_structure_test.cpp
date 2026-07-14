#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string header = read_source("include/cgpui/ui/test_app.hpp");
  const std::string internal = read_source("src/ui/test_app_internal.hpp");
  const std::string platform = read_source("src/ui/test_app_platform.cpp");
  const std::string renderer = read_source("src/ui/test_app_renderer.cpp");
  const std::string rendering = read_source("src/ui/test_app_rendering.cpp");
  const std::string runtime =
      read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string behavior =
      read_source("tests/ui/test_app_rendering_control_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_test_app_async_control_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string vocabulary =
      read_source("docs/gpui-public-authoring-vocabulary.md");
  const std::string core = read_source("docs/gpui-core-api-parity.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &header, &internal, &platform, &renderer, &rendering, &runtime, &behavior,
      &previous, &xmake, &vocabulary, &core, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* item : required) if (item->empty()) return 1;

  if (!contains(header, "struct TestWindowRenderSnapshot") ||
      !contains(header, "simulate_resize(Size framebuffer_size") ||
      !contains(header, "Result<void> try_draw_frame() const") ||
      !contains(header, "TestWindowRenderSnapshot render_snapshot() const")) {
    return 2;
  }
  if (!contains(internal, "TestWindowRenderSnapshot snapshot_") ||
      !contains(renderer, "snapshot_.begin_frame_count += 1") ||
      !contains(renderer, "snapshot_->draw_rect_count += 1") ||
      !contains(platform, "framebuffer_size_ = resized->size") ||
      !contains(platform, "callback_(WindowRedrawRequested{})")) return 3;
  if (!contains(rendering, "try_draw_frame_for_record(*record, *view)") ||
      !contains(rendering, "TestAppWindow::request_redraw() const") ||
      !contains(runtime, "Result<void> WindowRuntime::try_draw_frame_for_record(")) {
    return 4;
  }
  if (!contains(behavior, "First Render") ||
      !contains(behavior, "Second Render") ||
      !contains(behavior, "first.simulate_resize(") ||
      !contains(behavior, "first.render_snapshot()") ||
      !contains(behavior, "second.try_draw_frame()")) return 5;
  if (!contains(previous, "Step 674 WSL full-debug verification") ||
      !contains(xmake,
                "target(\"phase_g_test_app_rendering_control_test\")") ||
      !contains(xmake,
                "target(\"phase_g_test_app_rendering_control_structure_test\")")) {
    return 6;
  }
  if (line_count(header) > 125 || line_count(internal) > 140 ||
      line_count(renderer) > 45 || line_count(rendering) > 70 ||
      line_count(behavior) > 85) return 7;

  constexpr const char* completion =
      "Phase G Step 663 adds per-window TestApp rendering control with resize "
      "and redraw simulation, fallible and throwing frame draws, private "
      "renderer/frame snapshots, real additional-window rendering, and cross-"
      "window counter isolation. Step 664 GPUI-style platform service fake "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(vocabulary, "`TestAppWindow::simulate_resize(...)`") ||
      !contains(vocabulary, "`TestAppWindow::try_draw_frame()`") ||
      !contains(core, "Per-window `TestAppWindow` rendering control") ||
      !contains(ledger_json, "\"phase_g_step_663_sources\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 674 WSL full-debug verification\"")) return 9;
  return 0;
}
