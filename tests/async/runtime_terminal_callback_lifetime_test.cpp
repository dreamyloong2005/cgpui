#include "popup_test_support.hpp"
#include "cgpui/ui/async_io_hook.hpp"

#include <iostream>
#include <memory>

namespace {
struct State { int destroyed = 0; bool reenter = false; };
struct Capture {
  explicit Capture(std::shared_ptr<State> state) : state(std::move(state)) {}
  std::shared_ptr<State> state;
  std::function<void()> on_destroy;
  ~Capture() { ++state->destroyed; if (state->reenter && on_destroy) on_destroy(); }
};

int exercise() {
  popup_test::Application app;
  int views = 0;
  popup_test::CountingView view(views);
  popup_test::Renderer renderer(true);
  cgpui::WindowRuntime runtime(app, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        return &renderer;
      });
  for (const int mode : {0, 1, 2}) {
    const bool cancel = mode != 0;
    int called = 0;
    auto state = std::make_shared<State>();
    cgpui::TaskHandle task;
    auto capture = std::make_shared<Capture>(state);
    capture->on_destroy = [&] { (void)task.complete(); (void)task.cancelled(); };
    const std::weak_ptr<Capture> weak = capture;
    task = runtime.spawn_task([capture, &called](const cgpui::WindowRuntimeContext&) { ++called; });
    capture.reset();
    state->reenter = true;
    bool operation_succeeded = true;
    if (mode == 2) operation_succeeded = runtime.complete_task(task.id());
    if (cancel) operation_succeeded = task.cancel() && operation_succeeded;
    else {
      operation_succeeded = runtime.complete_task(task.id());
      runtime.drain_task_completions();
    }
    state->reenter = false;
    if (!operation_succeeded) return 1;
    if (!weak.expired() || state->destroyed != 1 || called != (cancel ? 0 : 1)) return 3;
    if (cancel ? !task.cancelled() : !task.complete()) return 4;
  }
  {
    auto state = std::make_shared<State>();
    auto group = runtime.create_task_group();
    auto child = group.create_child_group();
    auto capture = std::make_shared<Capture>(state);
    capture->on_destroy = [&] { (void)group.cancelled(); (void)child.active_task_count(); };
    const std::weak_ptr<Capture> weak = capture;
    auto task = child.spawn_task([capture](const cgpui::WindowRuntimeContext&) {});
    capture.reset();
    state->reenter = true;
    const bool cancelled = group.cancel();
    state->reenter = false;
    if (!cancelled || !task.cancelled() || !weak.expired() || state->destroyed != 1) return 5;
  }
  for (const int mode : {0, 1, 2}) {
    const bool cancel = mode != 0;
    int called = 0;
    auto state = std::make_shared<State>();
    cgpui::AsyncIoHook hook;
    auto capture = std::make_shared<Capture>(state);
    capture->on_destroy = [&] { (void)hook.cancel(); (void)hook.notify({}); };
    const std::weak_ptr<Capture> weak = capture;
    hook = runtime.create_async_io_hook(
        [capture, &called](const cgpui::WindowRuntimeContext&, const cgpui::AsyncIoResult&) { ++called; });
    capture.reset();
    state->reenter = true;
    bool operation_succeeded = true;
    if (mode == 2) operation_succeeded = hook.notify({});
    if (cancel) operation_succeeded = hook.cancel() && operation_succeeded;
    else {
      operation_succeeded = hook.notify({});
      runtime.drain_async_io_completions();
    }
    state->reenter = false;
    if (!operation_succeeded) return 6;
    if (!weak.expired() || state->destroyed != 1 || called != (cancel ? 0 : 1)) return 8;
    if (cancel ? !hook.cancelled() : !hook.complete()) return 9;
  }
  {
    auto state = std::make_shared<State>();
    cgpui::AsyncIoHook detached;
    std::weak_ptr<Capture> weak;
    {
      cgpui::WindowRuntime teardown(app, view,
          [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
            return &renderer;
          });
      auto owned = std::make_shared<Capture>(state);
      owned->on_destroy = [&] { (void)detached.cancel(); (void)detached.notify({}); };
      weak = owned;
      detached = teardown.create_async_io_hook(
          [owned](const cgpui::WindowRuntimeContext&, const cgpui::AsyncIoResult&) {});
      owned.reset();
      state->reenter = true;
    }
    state->reenter = false;
    if (!weak.expired() || state->destroyed != 1 || !detached.cancelled()) return 10;
  }
  return 0;
}
} // namespace

int main() {
  const int result = exercise();
  if (result != 0) std::cerr << "terminal callback lifetime failed: " << result << '\n';
  return result;
}
