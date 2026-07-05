#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* test_context_frame_pump_fixture = nullptr;

void dispatch_test_context_frame_pump_event() {
  test_context_frame_pump_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'F',
      .action = cgpui::KeyAction::pressed});
}

class TestContextFramePumpView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    render_count += 1;
    last_render_frame_index = context.frame_index;
    return cgpui::into_element(cgpui::div().size(16.0F, 8.0F));
  }

  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  int render_count = 0;
  int paint_count = 0;
  int last_render_frame_index = -1;
  bool request_redraw_pumped_frame = false;
  bool draw_frame_pumped_frame_without_platform_request = false;
  bool diagnostics_followed_pumped_frames = false;
};

int test_test_context_pumps_redraw_frames_through_runtime() {
  RuntimeFixture fixture;
  TestContextFramePumpView view;
  test_context_frame_pump_fixture = &fixture;
  fixture.app.on_run = &dispatch_test_context_frame_pump_event;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        const cgpui::TestContextCapability test_context =
            context.test_context();

        test_context.request_redraw();
        view.request_redraw_pumped_frame =
            fixture.window.request_redraw_count == 1 &&
            fixture.renderer.begin_frame_count == 1 &&
            fixture.frame.present_count == 1 && view.render_count == 1 &&
            view.paint_count == 1;

        const int redraw_count_after_request =
            fixture.window.request_redraw_count;
        test_context.draw_frame();
        view.draw_frame_pumped_frame_without_platform_request =
            fixture.window.request_redraw_count == redraw_count_after_request &&
            fixture.renderer.begin_frame_count == 2 &&
            fixture.frame.present_count == 2 && view.render_count == 2 &&
            view.paint_count == 2;

        const cgpui::RuntimeDiagnosticsSnapshot diagnostics =
            test_context.diagnostics_snapshot();
        view.diagnostics_followed_pumped_frames =
            diagnostics.frame_index == 2 &&
            diagnostics.last_render_record.has_value() &&
            diagnostics.last_render_record->statistics.has_value() &&
            diagnostics.last_render_record->statistics->frame_index == 2;
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  test_context_frame_pump_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!view.request_redraw_pumped_frame) {
    return 2;
  }
  if (!view.draw_frame_pumped_frame_without_platform_request) {
    return 3;
  }
  if (!view.diagnostics_followed_pumped_frames) {
    return 4;
  }
  if (view.last_render_frame_index != 1) {
    return 5;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_pumps_redraw_frames_through_runtime();
}
