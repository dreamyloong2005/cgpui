#include "window_runtime_test_support.hpp"

#include <thread>
#include <vector>

namespace {

class AsyncTimerView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::thread::id runtime_thread;
  std::vector<int> callback_order;
  bool callbacks_on_runtime_thread = true;
  bool integration_matches = false;
};

RuntimeFixture* async_timer_fixture = nullptr;
cgpui::WindowRuntime* async_timer_runtime = nullptr;
AsyncTimerView* async_timer_view = nullptr;

cgpui::TimerCallback timer_callback(int value) {
  return [value](const cgpui::WindowRuntimeContext&) {
    auto& view = *async_timer_view;
    view.callbacks_on_runtime_thread =
        view.callbacks_on_runtime_thread &&
        std::this_thread::get_id() == view.runtime_thread;
    view.callback_order.push_back(value);
  };
}

void dispatch_async_timer_sequence() {
  auto& fixture = *async_timer_fixture;
  auto& runtime = *async_timer_runtime;
  auto& view = *async_timer_view;
  view.runtime_thread = std::this_thread::get_id();

  const cgpui::TimerId long_timer =
      runtime.schedule_timer(50, timer_callback(50));
  const cgpui::TimerId one_shot =
      runtime.schedule_timer(20, timer_callback(20));
  const cgpui::TimerId repeating =
      runtime.schedule_repeating_timer(10, timer_callback(10));
  const bool deadlines_match =
      fixture.app.delayed_wakeup_delays ==
      std::vector<std::uint64_t>({50, 20, 10});

  fixture.app.advance_monotonic_time(10);
  fixture.app.dispatch_delayed_wakeup();
  const bool first_tick = view.callback_order == std::vector<int>({10});
  fixture.app.advance_monotonic_time(10);
  fixture.app.dispatch_delayed_wakeup();
  const bool second_tick =
      view.callback_order == std::vector<int>({10, 20, 10});
  fixture.app.advance_monotonic_time(10);
  fixture.app.dispatch_delayed_wakeup();
  const bool third_tick =
      view.callback_order == std::vector<int>({10, 20, 10, 10});

  const bool cancelled =
      runtime.cancel_timer(long_timer) && runtime.cancel_timer(repeating) &&
      !runtime.cancel_timer(one_shot);
  const std::size_t cancellations_before =
      fixture.app.cancel_delayed_wakeup_count;
  const cgpui::TimerId immediate =
      runtime.schedule_timer(0, timer_callback(0));
  const bool immediate_requested =
      immediate.value != 0 && fixture.app.wakeup_pending;
  fixture.app.dispatch_wakeup();

  view.integration_matches =
      long_timer.value != 0 && one_shot.value != 0 &&
      repeating.value != 0 && deadlines_match && first_tick && second_tick &&
      third_tick && cancelled && cancellations_before >= 1 &&
      immediate_requested &&
      view.callback_order == std::vector<int>({10, 20, 10, 10, 0}) &&
      fixture.app.cancel_delayed_wakeup_count > cancellations_before;
}

} // namespace

int main() {
  RuntimeFixture fixture;
  AsyncTimerView view;
  fixture.app.on_run = &dispatch_async_timer_sequence;
  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  async_timer_fixture = &fixture;
  async_timer_runtime = &runtime;
  async_timer_view = &view;
  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  async_timer_fixture = nullptr;
  async_timer_runtime = nullptr;
  async_timer_view = nullptr;
  if (result != 0) return 1;
  if (!view.callbacks_on_runtime_thread) return 2;
  return view.integration_matches ? 0 : 3;
}
