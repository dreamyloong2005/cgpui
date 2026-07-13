#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <memory>
#include <variant>

namespace {

class TimerView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }
    immediate = context.runtime.schedule_timer(
        0, [this](const cgpui::WindowRuntimeContext&) { immediate_count += 1; });
    future = context.runtime.schedule_timer(
        10,
        [this](const cgpui::WindowRuntimeContext& timer_context) {
          future_count += 1;
          (void)timer_context.runtime.schedule_timer(
              0,
              [this](const cgpui::WindowRuntimeContext&) { nested_count += 1; });
        });
    repeating = context.runtime.schedule_repeating_timer(
        5,
        [this](const cgpui::WindowRuntimeContext&) { repeating_count += 1; });
    return cgpui::EventResult::consumed_event();
  }

  cgpui::TimerId immediate{};
  cgpui::TimerId future{};
  cgpui::TimerId repeating{};
  int immediate_count = 0;
  int future_count = 0;
  int nested_count = 0;
  int repeating_count = 0;
};

} // namespace

int main() {
  cgpui::TestApp app;
  cgpui::TestAppWindow window = app.open_window(
      cgpui::WindowOptions{}, std::make_unique<TimerView>());
  TimerView* view = window.root_view_as<TimerView>();
  if (view == nullptr) return 1;

  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'T'});
  app.run_until_parked();
  if (view->immediate_count != 1 || view->future_count != 0 ||
      view->nested_count != 0 || view->repeating_count != 0) {
    return 2;
  }

  app.advance_time(5);
  if (view->future_count != 0 || view->repeating_count != 1) return 3;

  app.advance_time_until_parked(5);
  if (view->future_count != 1 || view->nested_count != 1 ||
      view->repeating_count != 2) {
    return 4;
  }

  if (!app.cancel_timer(view->repeating) ||
      app.cancel_timer(view->repeating)) {
    return 5;
  }
  app.advance_time_until_parked(100);
  if (view->repeating_count != 2 || app.cancel_timer(view->immediate) ||
      app.cancel_timer(view->future)) {
    return 6;
  }
  return 0;
}
