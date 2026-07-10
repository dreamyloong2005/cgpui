#include "window_runtime_test_support.hpp"

#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  std::ifstream source(source_root() + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }
  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

int test_render_time_invalidations_schedule_one_next_frame() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int after_render_count = 0;
  std::vector<cgpui::InvalidationState> after_frame_states;
  runtime.set_after_render_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::RenderRecord&) {
        ++after_render_count;
        if (context.frame_index == 0) {
          context.request_layout();
          context.request_paint();
          context.request_paint();
        }
      });
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        after_frame_states.push_back(context.invalidation_state());
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  if (result != 0) {
    return 10;
  }
  if (fixture.window.request_redraw_count != 2 ||
      fixture.renderer.begin_frame_count != 2 ||
      fixture.view.paint_count != 2 || fixture.frame.present_count != 2 ||
      after_render_count != 2 || after_frame_states.size() != 2) {
    return 11;
  }
  if (!after_frame_states[0].layout || !after_frame_states[0].paint ||
      after_frame_states[0].render || after_frame_states[1].render ||
      after_frame_states[1].layout || after_frame_states[1].paint) {
    return 12;
  }
  const cgpui::InvalidationState final_state = runtime.invalidation_state();
  return !final_state.render && !final_state.layout && !final_state.paint
             ? 0
             : 13;
}

int test_after_frame_invalidation_schedules_one_next_frame() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  int after_frame_count = 0;
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        ++after_frame_count;
        if (context.frame_index == 1) {
          context.request_render();
          context.request_paint();
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  return result == 0 && fixture.window.request_redraw_count == 2 &&
                 fixture.renderer.begin_frame_count == 2 &&
                 fixture.view.paint_count == 2 &&
                 fixture.frame.present_count == 2 && after_frame_count == 2
             ? 0
             : 20;
}

int test_structure_and_documentation() {
  const std::string scheduling =
      read_source("src/ui/runtime_frame_scheduling.cpp");
  const std::string redraw = read_source("src/ui/runtime_scheduling.cpp");
  const std::string frame_results =
      read_source("src/ui/runtime_renderer_frame_results.cpp");
  const std::string internal =
      read_source("src/ui/window_runtime_internal.hpp");
  const std::string scheduling_header =
      read_source("src/ui/runtime_frame_scheduling_internal.hpp");
  const std::string run = read_source("src/ui/runtime_run.cpp");
  const std::string structure =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  if (scheduling.empty() || redraw.empty() || frame_results.empty() ||
      internal.empty() || scheduling_header.empty() || run.empty() ||
      structure.empty()) {
    return 30;
  }
  if (!contains(scheduling, "WindowRuntime::begin_frame_scheduling()") ||
      !contains(scheduling, "WindowRuntime::complete_frame_scheduling()") ||
      !contains(scheduling, "WindowRuntime::abort_frame_scheduling()") ||
      !contains(redraw, "next_frame_redraw_requested_") ||
      !contains(redraw, "rendering_frame_") ||
      !contains(frame_results, "begin_frame_scheduling()") ||
      !contains(frame_results, "complete_frame_scheduling()") ||
      !contains(frame_results, "abort_frame_scheduling()") ||
      !contains(internal,
                "#include \"runtime_frame_scheduling_internal.hpp\"") ||
      !contains(scheduling_header, "bool rendering_frame_ = false") ||
      !contains(scheduling_header,
                "bool next_frame_redraw_requested_ = false") ||
      !contains(run, "rendering_frame_ = false") ||
      !contains(run, "next_frame_redraw_requested_ = false") ||
      !contains(structure, "runtime_frame_scheduling.cpp") ||
      contains(scheduling, "render_view(")) {
    return 31;
  }

  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const char* required[] = {
      "Phase E Step 513",
      "next-frame scheduling",
      "render-time invalidation survives frame completion",
      "coalesced into exactly one platform redraw",
      "Step 514 batching and scheduling closeout",
  };
  for (const char* value : required) {
    if (!contains(roadmap, value) || !contains(ledger_md, value) ||
        !contains(ledger_json, value) || !contains(task_plan, value) ||
        !contains(findings, value)) {
      return 40;
    }
  }
  return 0;
}

} // namespace

int main() {
  if (const int result =
          test_render_time_invalidations_schedule_one_next_frame();
      result != 0) {
    return result;
  }
  if (const int result =
          test_after_frame_invalidation_schedules_one_next_frame();
      result != 0) {
    return result;
  }
  return test_structure_and_documentation();
}
