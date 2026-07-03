#include "window_runtime_test_support.hpp"

namespace {

int test_window_runtime_smoke_runs_initial_redraw() {
  RuntimeFixture fixture;
  fixture.app.on_run = +[] {};

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{
      .title = "Runtime Smoke",
      .size = {.width = 640.0F, .height = 480.0F},
  });
  if (result != 0) {
    return 1;
  }
  if (fixture.app.create_window_count != 1 || fixture.app.run_count != 1) {
    return 2;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 || fixture.frame.draw_count != 1) {
    return 3;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_window_runtime_smoke_runs_initial_redraw();
      result != 0) {
    return result;
  }
  return 0;
}
