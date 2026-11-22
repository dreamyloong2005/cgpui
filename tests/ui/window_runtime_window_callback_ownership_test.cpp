#include "runtime_window_lifecycle_test_support.hpp"

#include <iostream>

namespace {
using namespace runtime_window_test;

int moved_task_groups_keep_the_window_and_release_skipped_completions() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>(app.last_created));
        return renderers.back().get();
      });
  int failure = 0;
  app.on_run = [&] {
    // This case checks move/origin binding. Late TaskGroup destruction after
    // runtime shutdown is covered separately by runtime_task_group_outlive_test.
    cgpui::TaskGroup origin;
    auto view = std::make_unique<View>();
    view->on_event = [&](const cgpui::PlatformEvent&, const cgpui::WindowRuntimeContext& context) {
      origin = context.create_task_group();
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime.try_open_window(popup_options(), std::move(view));
    if (!opened) { failure = 1; return; }
    const auto native = app.children.back();
    native->emit(key());
    const auto group_id = origin.id();
    cgpui::TaskGroup moved(std::move(origin));
    cgpui::TaskGroup destination;
    destination = std::move(moved);
    auto child_group = destination.create_child_group();
    if (group_id.value == 0 || destination.id() != group_id || origin.id().value != 0 ||
        moved.id().value != 0 || child_group.id().value == 0) { failure = 2; return; }

    int active_calls = 0;
    bool context_matches = false;
    const auto active = child_group.spawn_task(cgpui::TaskPriority::high,
        [&, window_id = opened->runtime_id, view_id = opened->root_view_id,
         native_address = native->native_address](const cgpui::WindowRuntimeContext& context) {
          ++active_calls;
          context_matches = context.window_runtime_id == window_id && context.view_id == view_id &&
              &context.platform_window == native_address;
          context.request_render();
        });
    if (!runtime.complete_task(active.id())) { failure = 3; return; }
    app.wake();
    if (active_calls != 1 || !context_matches || !active.complete() ||
        !child_group.complete() || !destination.complete() ||
        native->redraws == 0 || app.root->redraws != 0) { failure = 4; return; }

    int late_calls = 0;
    auto capture = std::make_shared<int>(17);
    const std::weak_ptr<int> weak = capture;
    const auto queued = child_group.spawn_task(
        [capture, &late_calls](const cgpui::WindowRuntimeContext&) { ++late_calls; });
    capture.reset();
    if (weak.expired() || !runtime.complete_task(queued.id()) ||
        !cgpui::close_popup(runtime, opened->runtime_id)) { failure = 5; return; }
    app.wake();
    if (late_calls != 0 || !queued.complete() || queued.cancelled() ||
        !child_group.complete() || !destination.complete() ||
        !weak.expired() || app.root->redraws != 0) failure = 6;
  };
  return runtime.run({}, {.request_initial_redraw = false}) == 0 ? failure : 7;
}

int closing_window_cancels_and_releases_its_repeating_timer() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>(app.last_created));
        return renderers.back().get();
      });
  int failure = 0;
  app.on_run = [&] {
    cgpui::TimerId timer;
    std::weak_ptr<int> weak;
    int calls = 0;
    bool context_matches = false;
    cgpui::AppOpenedWindow opened;
    auto view = std::make_unique<View>();
    view->on_event = [&](const cgpui::PlatformEvent&, const cgpui::WindowRuntimeContext& context) {
      auto capture = std::make_shared<int>(23);
      weak = capture;
      timer = context.schedule_repeating_timer(5,
          [&, capture, window_id = opened.runtime_id, view_id = opened.root_view_id,
           native_address = app.children.back()->native_address](
              const cgpui::WindowRuntimeContext& later) {
            ++calls;
            context_matches = later.window_runtime_id == window_id && later.view_id == view_id &&
                &later.platform_window == native_address;
            later.request_render();
          });
      return cgpui::EventResult::consumed_event();
    };
    const auto result = runtime.try_open_window(popup_options(), std::move(view));
    if (!result) { failure = 10; return; }
    opened = *result;
    const auto native = app.children.back();
    native->emit(key());
    if (timer.value == 0 || weak.expired()) { failure = 11; return; }
    runtime.advance_time(5);
    app.wake();
    if (calls != 1 || !context_matches || native->redraws == 0 || app.root->redraws != 0) {
      failure = 12;
      return;
    }
    if (!cgpui::close_popup(runtime, opened.runtime_id) || !weak.expired()) {
      failure = 13;
      return;
    }
    runtime.advance_time(50);
    app.wake();
    if (calls != 1 || runtime.cancel_timer(timer) || !native->destroyed ||
        native->destroyed_during_callback || app.root->redraws != 0) failure = 14;
  };
  return runtime.run({}, {.request_initial_redraw = false}) == 0 ? failure : 15;
}
} // namespace

int main() {
  for (const auto test : {moved_task_groups_keep_the_window_and_release_skipped_completions,
                          closing_window_cancels_and_releases_its_repeating_timer}) {
    if (const int result = test(); result != 0) {
      std::cerr << "window callback ownership failure: " << result << '\n';
      return result;
    }
  }
  return 0;
}
