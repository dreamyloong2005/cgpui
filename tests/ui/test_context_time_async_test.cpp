#include "window_runtime_test_support.hpp"

namespace {

RuntimeFixture* test_context_time_async_fixture = nullptr;

void dispatch_test_context_time_async_event() {
  test_context_time_async_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'T',
      .action = cgpui::KeyAction::pressed});
}

class TestContextTimeAsyncView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  int task_count = 0;
  int nested_task_count = 0;
  int timer_count = 0;
  int future_timer_count = 0;
  int deferred_count = 0;
  int paint_count = 0;
  bool run_until_parked_drained_ready_work = false;
  bool run_until_parked_left_future_timer_pending = false;
  bool advance_time_until_parked_drained_future_timer = false;
};

int test_test_context_runs_until_parked_without_advancing_future_timers() {
  RuntimeFixture fixture;
  TestContextTimeAsyncView view;
  test_context_time_async_fixture = &fixture;
  fixture.app.on_run = &dispatch_test_context_time_async_event;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        const cgpui::TaskHandle first_task = context.runtime.spawn_task(
            [&](const cgpui::WindowRuntimeContext& task_context) {
              view.task_count += 1;
              task_context.defer(
                  [&](const cgpui::WindowRuntimeContext& deferred_context) {
                    view.deferred_count += 1;
                    const cgpui::TaskHandle nested_task =
                        deferred_context.runtime.spawn_task(
                            [&](const cgpui::WindowRuntimeContext&) {
                              view.nested_task_count += 1;
                            });
                    (void)deferred_context.runtime.complete_task(
                        nested_task.id());
                  });
            });
        (void)context.runtime.complete_task(first_task.id());
        (void)context.runtime.schedule_timer(
            0,
            [&](const cgpui::WindowRuntimeContext& timer_context) {
              view.timer_count += 1;
              timer_context.request_paint();
            });
        (void)context.runtime.schedule_timer(
            10,
            [&](const cgpui::WindowRuntimeContext&) {
              view.future_timer_count += 1;
            });

        const cgpui::TestContextCapability test_context =
            context.test_context();
        test_context.run_until_parked();
        view.run_until_parked_drained_ready_work =
            view.task_count == 1 && view.nested_task_count == 1 &&
            view.timer_count == 1 && view.deferred_count == 1;
        view.run_until_parked_left_future_timer_pending =
            view.future_timer_count == 0;

        test_context.advance_time_until_parked(10);
        view.advance_time_until_parked_drained_future_timer =
            view.future_timer_count == 1;
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  test_context_time_async_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!view.run_until_parked_drained_ready_work) {
    return 2;
  }
  if (!view.run_until_parked_left_future_timer_pending) {
    return 3;
  }
  if (!view.advance_time_until_parked_drained_future_timer) {
    return 4;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 || view.paint_count != 1) {
    return 5;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_runs_until_parked_without_advancing_future_timers();
}
