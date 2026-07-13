#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/window_runtime.hpp"

#include <memory>
#include <variant>
#include <vector>

namespace {

class AsyncView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr) return cgpui::EventResult::unhandled();
    if (key->key_code == 'P') {
      parked = context.runtime.spawn_task(
          [this](const cgpui::WindowRuntimeContext&) { order.push_back(6); });
      return cgpui::EventResult::consumed_event();
    }
    low = context.runtime.spawn_task(
        cgpui::TaskPriority::low,
        [this](const cgpui::WindowRuntimeContext&) { order.push_back(4); });
    normal = context.runtime.spawn_task(
        cgpui::TaskPriority::normal,
        [this](const cgpui::WindowRuntimeContext&) { order.push_back(2); });
    normal_second = context.runtime.spawn_task(
        cgpui::TaskPriority::normal,
        [this](const cgpui::WindowRuntimeContext&) { order.push_back(3); });
    high = context.runtime.spawn_task(
        cgpui::TaskPriority::high,
        [this](const cgpui::WindowRuntimeContext& task_context) {
          order.push_back(1);
          nested = task_context.runtime.spawn_task(
              [this](const cgpui::WindowRuntimeContext&) { order.push_back(5); });
          (void)task_context.runtime.complete_task(nested.id());
        });
    return cgpui::EventResult::consumed_event();
  }

  cgpui::TaskHandle low;
  cgpui::TaskHandle normal;
  cgpui::TaskHandle normal_second;
  cgpui::TaskHandle high;
  cgpui::TaskHandle nested;
  cgpui::TaskHandle parked;
  std::vector<int> order;
};

} // namespace

int main() {
  cgpui::TestApp app;
  cgpui::TestAppWindow window = app.open_window(
      cgpui::WindowOptions{}, std::make_unique<AsyncView>());
  AsyncView* view = window.root_view_as<AsyncView>();
  if (view == nullptr) return 1;

  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'A'});
  if (!app.complete_task(view->low.id()) ||
      !app.complete_task(view->high.id()) ||
      !app.complete_task(view->normal.id()) ||
      !app.complete_task(view->normal_second.id()) || !view->order.empty()) {
    return 2;
  }
  app.drain_task_completions();
  if (view->order != std::vector<int>{1, 2, 3, 4, 5} ||
      !view->high.complete() || !view->normal.complete() ||
      !view->normal_second.complete() || !view->low.complete() ||
      !view->nested.complete()) {
    return 3;
  }
  if (app.complete_task(view->high.id()) || app.complete_task(cgpui::TaskId{}) ||
      app.complete_task(cgpui::TaskId{999999})) {
    return 4;
  }

  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'P'});
  if (!app.complete_task(view->parked.id())) return 5;
  app.run_until_parked();
  if (view->order != std::vector<int>{1, 2, 3, 4, 5, 6} ||
      !view->parked.complete()) {
    return 6;
  }
  return 0;
}
