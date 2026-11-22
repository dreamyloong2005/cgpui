#include "runtime_window_lifecycle_test_support.hpp"

#include <array>
#include <iostream>

namespace {
using namespace runtime_window_test;

enum CallbackKind : std::size_t { deferred, timer, task, priority_task, io, batch, callback_count };

struct CallbackState {
  cgpui::WindowRuntimeId window_id;
  cgpui::ViewId view_id;
  cgpui::PlatformWindow* native = nullptr;
  std::array<int, callback_count> calls{};
  cgpui::TaskHandle foreground;
  cgpui::TaskHandle prioritized;
  cgpui::AsyncIoHook hook;
  bool context_matches = true;
  bool queued = true;
  bool io_succeeded = false;
};

cgpui::DeferredCallback callback(CallbackState& state, CallbackKind kind) {
  // Retain only stable expected identities and test state, never the source
  // event's WindowRuntimeContext (which is a callback-local temporary).
  return [&state, kind](const cgpui::WindowRuntimeContext& context) {
    ++state.calls[kind];
    state.context_matches = state.context_matches &&
        context.window_runtime_id == state.window_id && context.view_id == state.view_id &&
        &context.platform_window == state.native &&
        context.current_window().runtime_id() == state.window_id;
    context.request_render();
  };
}

void enqueue(const cgpui::WindowRuntimeContext& context, CallbackState& state,
             std::uint64_t timer_delay) {
  context.defer(callback(state, deferred));
  state.queued = context.schedule_timer(timer_delay, callback(state, timer)).value != 0;
  state.foreground = context.spawn_task(callback(state, task));
  state.prioritized = context.spawn_task(cgpui::TaskPriority::high, callback(state, priority_task));
  state.hook = context.create_async_io_hook(
      [&state, completed = callback(state, io)](
          const cgpui::WindowRuntimeContext& later, const cgpui::AsyncIoResult& result) {
        state.io_succeeded = result.has_value();
        completed(later);
      });
  state.queued = context.runtime.complete_task(state.foreground.id()) && state.queued;
  state.queued = context.runtime.complete_task(state.prioritized.id()) && state.queued;
  state.queued = state.hook.notify({}) && state.queued;
}

bool calls_are(const CallbackState& state, int expected) {
  for (const int calls : state.calls) if (calls != expected) return false;
  return true;
}

bool terminal(const CallbackState& state) {
  return state.foreground.complete() && !state.foreground.cancelled() &&
      state.prioritized.complete() && !state.prioritized.cancelled() &&
      state.hook.complete() && !state.hook.cancelled();
}

int child_callbacks_keep_the_originating_window() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  CallbackState state;
  cgpui::WindowRuntime runtime(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>(app.last_created));
        return renderers.back().get();
      });
  int failure = 0;
  app.on_run = [&] {
    auto view = std::make_unique<View>();
    view->on_event = [&](const cgpui::PlatformEvent&, const cgpui::WindowRuntimeContext& context) {
      enqueue(context, state, 0);
      context.batch_updates(callback(state, batch));
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime.try_open_window(cgpui::WindowOptions{}.title("callback child"),
                                                std::move(view));
    if (!opened) { failure = 1; return; }
    const auto native = app.children.back();
    state.window_id = opened->runtime_id;
    state.view_id = opened->root_view_id;
    state.native = native->native_address;
    native->emit(key());
    app.wake();
    if (!state.queued || !state.context_matches || !state.io_succeeded ||
        !calls_are(state, 1) || !terminal(state)) { failure = 2; return; }
    // Implementations may coalesce redraws across these completions.
    if (native->redraws == 0 || app.root->redraws != 0) failure = 3;
  };
  return runtime.run({}, {.request_initial_redraw = false}) == 0 ? failure : 4;
}

int closing_child_skips_its_callbacks_without_affecting_root() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  CallbackState child_callbacks;
  CallbackState root_callbacks;
  cgpui::WindowRuntime runtime(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<Renderer>(app.last_created));
        return renderers.back().get();
      });
  int failure = 0;
  root.on_event = [&](const cgpui::PlatformEvent&, const cgpui::WindowRuntimeContext& context) {
    enqueue(context, root_callbacks, 0);
    context.batch_updates(callback(root_callbacks, batch));
    return cgpui::EventResult::consumed_event();
  };
  app.on_run = [&] {
    auto view = std::make_unique<View>();
    view->on_event = [&](const cgpui::PlatformEvent&, const cgpui::WindowRuntimeContext& context) {
      enqueue(context, child_callbacks, 10);
      if (!cgpui::close_popup(runtime, child_callbacks.window_id)) failure = 10;
      context.batch_updates(callback(child_callbacks, batch));
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime.try_open_window(popup_options(), std::move(view));
    if (!opened) { failure = 11; return; }
    const auto native = app.children.back();
    child_callbacks.window_id = opened->runtime_id;
    child_callbacks.view_id = opened->root_view_id;
    child_callbacks.native = native->native_address;
    native->emit(key());
    if (failure != 0) return;
    app.wake();
    runtime.advance_time(10);
    app.wake();
    if (!child_callbacks.queued || !calls_are(child_callbacks, 0) ||
        !terminal(child_callbacks) || !native->destroyed || native->destroyed_during_callback ||
        runtime.window_runtime_record(opened->runtime_id) != nullptr ||
        native->redraws != 0 || app.root->redraws != 0) { failure = 12; return; }

    root_callbacks.window_id = runtime.root_window_runtime_id();
    root_callbacks.view_id = runtime.window_runtime_record(root_callbacks.window_id)->root_view_id;
    root_callbacks.native = app.root->native_address;
    app.root->emit(key());
    app.wake();
    if (!root_callbacks.queued || !root_callbacks.context_matches ||
        !root_callbacks.io_succeeded || !calls_are(root_callbacks, 1) ||
        !terminal(root_callbacks) || app.root->redraws == 0 || native->redraws != 0) failure = 13;
  };
  return runtime.run({}, {.request_initial_redraw = false}) == 0 ? failure : 14;
}

int manual_context_without_window_id_keeps_root_compatibility() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  CallbackState state;
  cgpui::WindowRuntime runtime(app, root, [&](const auto&) -> cgpui::Result<cgpui::Renderer*> {
    renderers.push_back(std::make_unique<Renderer>(app.last_created));
    return renderers.back().get();
  });
  int failure = 0;
  app.on_run = [&] {
    state.window_id = runtime.root_window_runtime_id();
    const auto* record = runtime.window_runtime_record(state.window_id);
    state.view_id = record->root_view_id;
    state.native = record->window;
    const cgpui::WindowRuntimeContext manual{
        .runtime = runtime, .application = app, .platform_window = *record->window,
        .renderer = *record->renderer, .view_id = record->root_view_id,
        .viewport_size = record->viewport_size, .scale = record->scale};
    if (manual.window_runtime_id.value != 0 ||
        manual.current_window().runtime_id() != state.window_id ||
        manual.window().root_view_id() != state.view_id || !manual.current_window().active() ||
        manual.window_context().runtime_id() != state.window_id ||
        manual.window_context().root_view_id() != state.view_id ||
        !manual.window_context().active()) { failure = 20; return; }
    for (int mode = 0; mode < 3; ++mode) {
      manual.clear_invalidation();
      const int before = app.root->redraws;
      if (mode == 0) manual.request_render();
      else if (mode == 1) manual.request_layout();
      else manual.request_paint();
      const auto invalidation = runtime.invalidation_state();
      if (invalidation.render != (mode == 0) || invalidation.layout != (mode != 2) ||
          !invalidation.paint || app.root->redraws != before + 1) { failure = 21; return; }
    }
    // Only aggregate context compatibility maps zero to root, not an explicit invalid Window.
    const cgpui::Window invalid(runtime, {});
    runtime.clear_invalidation();
    const int before = app.root->redraws;
    invalid.request_render(); invalid.request_layout(); invalid.request_paint();
    const auto invalidation = runtime.invalidation_state();
    if (invalid.active() || invalid.runtime_id().value != 0 ||
        invalidation.render || invalidation.layout || invalidation.paint ||
        app.root->redraws != before) { failure = 22; return; }
    enqueue(manual, state, 0);
    manual.batch_updates(callback(state, batch));
    app.wake();
    if (!state.queued || !state.context_matches || !state.io_succeeded ||
        !calls_are(state, 1) || !terminal(state) || app.root->redraws <= before) failure = 23;
  };
  return runtime.run({}, {.request_initial_redraw = false}) == 0 ? failure : 24;
}
} // namespace

int main() {
  for (const auto test : {child_callbacks_keep_the_originating_window,
                          closing_child_skips_its_callbacks_without_affecting_root,
                          manual_context_without_window_id_keeps_root_compatibility}) {
    if (const int result = test(); result != 0) {
      std::cerr << "window callback context failure: " << result << '\n';
      return result;
    }
  }
  return 0;
}
