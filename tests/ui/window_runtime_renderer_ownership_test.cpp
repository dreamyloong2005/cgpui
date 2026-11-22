#include "runtime_window_lifecycle_test_support.hpp"

#include <iostream>

namespace {
using namespace runtime_window_test;

bool renderer_released_before_window(const NativeState& state) {
  return state.destroyed && !state.destroyed_during_callback &&
      !state.renderer_destroyed_after_window && state.live_renderers == 0 &&
      state.renderer_destroy_order != 0 &&
      state.renderer_destroy_order < state.window_destroy_order;
}

cgpui::AppRunnerOptions setup(cgpui::WindowRuntime*& runtime) {
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&runtime](cgpui::WindowRuntime& value) { runtime = &value; };
  return options;
}

int child_close_releases_owned_renderers() {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  int failure = 0;
  int destroyed_views = 0;
  app.on_run = [&] {
    for (int index = 0; index < 16 && failure == 0; ++index) {
      auto view = std::make_unique<View>();
      view->on_destroy = [&] { ++destroyed_views; };
      const bool popup = index % 2 != 0;
      auto opened = runtime->try_open_window(
          popup ? popup_options() : cgpui::WindowOptions{}.title("owned child"),
          std::move(view));
      if (!opened) { failure = 1; return; }
      auto native = app.children.back();
      const auto* record = runtime->window_runtime_record(opened->runtime_id);
      if (record == nullptr || !record->owns_renderer || app.counters.live_renderers != 2) {
        failure = 2;
        return;
      }
      if (popup) {
        if (!cgpui::close_popup(*runtime, opened->runtime_id)) { failure = 3; return; }
      } else {
        native->emit(cgpui::WindowCloseRequested{});
      }
      app.wake();
      if (app.counters.live_renderers != 1 || destroyed_views != index + 1 ||
          !renderer_released_before_window(*native) ||
          runtime->window_runtime_record(opened->runtime_id) != nullptr ||
          !runtime->app_opened_windows().empty()) {
        failure = 4;
      }
    }
  };
  const int result = cgpui::run_app(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        return std::make_unique<Renderer>(app.last_created);
      }, setup(runtime));
  if (result != 0) return 5;
  if (failure != 0) return failure;
  return app.counters.created_renderers == 17 && app.counters.live_renderers == 0 &&
          app.counters.destroyed_renderers == 17 && renderer_released_before_window(*app.root)
      ? 0 : 6;
}

int shutdown_releases_renderers_before_native_windows() {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  int failure = 0;
  app.on_run = [&] {
    for (int index = 0; index < 2; ++index) {
      if (!runtime->try_open_window(cgpui::WindowOptions{}.title("live at shutdown"),
                                   std::make_unique<View>())) failure = 10;
    }
  };
  const int result = cgpui::run_app(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        return std::make_unique<Renderer>(app.last_created);
      }, setup(runtime));
  if (result != 0 || failure != 0) return 10;
  if (app.counters.live_renderers != 0 || app.counters.destroyed_renderers != 3 ||
      !renderer_released_before_window(*app.root)) return 11;
  for (const auto& child : app.children) {
    if (!renderer_released_before_window(*child)) return 12;
  }
  return 0;
}

enum class Failure { native_creation, renderer_creation, popup_refusal };

int failed_child_creation_releases_owned_state(Failure injected) {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  bool fail_child = true;
  int failure = 0;
  int destroyed_views = 0;
  app.on_run = [&] {
    app.fail_child_creation = injected == Failure::native_creation;
    auto view = std::make_unique<View>();
    view->on_destroy = [&] { ++destroyed_views; };
    const auto opened = runtime->try_open_window(popup_options(), std::move(view));
    const auto expected = injected == Failure::native_creation
        ? cgpui::ErrorCode::window_creation_failed
        : injected == Failure::renderer_creation
            ? cgpui::ErrorCode::renderer_initialization_failed
            : cgpui::ErrorCode::unsupported_platform;
    if (opened || opened.error().code != expected || destroyed_views != 1 ||
        app.counters.live_renderers != 1 || !runtime->app_opened_windows().empty() ||
        runtime->window_runtime_records().size() != 1) { failure = 20; return; }
    if (injected == Failure::native_creation) {
      if (!app.children.empty()) { failure = 21; return; }
    } else {
      const auto& native = *app.children.back();
      if (!native.destroyed || native.live_renderers != 0 ||
          (injected == Failure::popup_refusal && !renderer_released_before_window(native))) {
        failure = 22;
        return;
      }
    }
    fail_child = false;
    app.fail_child_creation = false;
    const auto recovered = runtime->try_open_window(popup_options(), std::make_unique<View>());
    if (!recovered || app.counters.live_renderers != 2 ||
        !cgpui::close_popup(*runtime, recovered->runtime_id)) { failure = 23; return; }
    app.wake();
    if (app.counters.live_renderers != 1 ||
        !renderer_released_before_window(*app.children.back())) failure = 24;
  };
  const int result = cgpui::run_app(app, root,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
        const bool child = app.last_created != app.root;
        if (child && fail_child && injected == Failure::renderer_creation) {
          return std::unexpected(cgpui::Error{
              .code = cgpui::ErrorCode::renderer_initialization_failed,
              .message = "injected child renderer failure"});
        }
        return std::make_unique<Renderer>(app.last_created,
            !(child && fail_child && injected == Failure::popup_refusal));
      }, setup(runtime));
  if (result != 0) return 25;
  if (failure != 0) return failure;
  return app.counters.live_renderers == 0 &&
          app.counters.created_renderers == app.counters.destroyed_renderers &&
          renderer_released_before_window(*app.root)
      ? 0 : 26;
}

int borrowed_renderers_remain_caller_owned() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> owners;
  int failure = 0;
  {
    cgpui::WindowRuntime runtime(app, root,
        [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
          owners.push_back(std::make_unique<Renderer>(app.last_created));
          return owners.back().get();
        });
    app.on_run = [&] {
      const auto opened = runtime.try_open_window(cgpui::WindowOptions{}, std::make_unique<View>());
      if (!opened) { failure = 30; return; }
      const auto* record = runtime.window_runtime_record(opened->runtime_id);
      if (record == nullptr || record->owns_renderer) { failure = 31; return; }
      app.children.back()->emit(cgpui::WindowCloseRequested{});
      app.wake();
      if (app.counters.live_renderers != 2 || app.counters.destroyed_renderers != 0) failure = 32;
    };
    if (runtime.run({}, {.request_initial_redraw = false}) != 0) return 33;
  }
  if (failure != 0) return failure;
  if (app.counters.live_renderers != 2 || app.counters.destroyed_renderers != 0) return 34;
  owners.clear();
  return app.counters.live_renderers == 0 && app.counters.destroyed_renderers == 2 ? 0 : 35;
}
} // namespace

int main() {
  for (const auto test : {child_close_releases_owned_renderers,
                          shutdown_releases_renderers_before_native_windows,
                          borrowed_renderers_remain_caller_owned}) {
    if (const int result = test(); result != 0) {
      std::cerr << "renderer ownership failure: " << result << '\n';
      return result;
    }
  }
  for (const Failure failure : {Failure::native_creation, Failure::renderer_creation,
                                Failure::popup_refusal}) {
    if (const int result = failed_child_creation_releases_owned_state(failure); result != 0) {
      std::cerr << "renderer rollback failure " << static_cast<int>(failure) << ": " << result << '\n';
      return result;
    }
  }
  return 0;
}
