#include "runtime_window_lifecycle_test_support.hpp"

#include <cstdio>

namespace {
using namespace runtime_window_test;

int self_close(bool render, bool deferred) {
  Application app;
  View root;
  cgpui::WindowRuntime* runtime = nullptr;
  bool view_destroyed = false;
  bool callback_alive = false;
  int failure = 0;
  app.on_run = [&] {
    auto view = std::make_unique<View>();
    view->on_destroy = [&] { view_destroyed = true; };
    const auto close = [&](const cgpui::WindowRuntimeContext& context) {
      const auto id = context.window_runtime_id;
      if (deferred) {
        context.defer([id](const auto& next) { (void)cgpui::close_popup(next.runtime, id); });
      } else if (!cgpui::close_popup(context.runtime, id)) failure = 1;
      callback_alive = !view_destroyed && !app.children.back()->destroyed;
    };
    if (render) view->on_render = close;
    else view->on_event = [&](const auto&, const auto& context) {
      close(context);
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime->open_window(popup_options(), std::move(view));
    const auto native = app.children.back();
    native->emit(render ? cgpui::PlatformEvent{cgpui::WindowRedrawRequested{}}
                        : cgpui::PlatformEvent{key()});
    if (!callback_alive || native->destroyed || native->destroyed_during_callback) failure = 2;
    if (render && !deferred && native->presents != 0) failure = 3;
    app.wake();
    // Deferred work may request a second safe collection wakeup.
    app.wake();
    if (!view_destroyed || !native->destroyed || native->destroyed_during_callback ||
        native->renderer_destroyed_after_window || native->live_renderers != 0 ||
        runtime->window_runtime_record(opened.runtime_id) != nullptr) failure = 4;
  };
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&](auto& value) { runtime = &value; };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    return std::make_unique<Renderer>(app.last_created);
  }, options);
  return result != 0 ? 5 : failure;
}
} // namespace

int main() {
  for (const bool render : {false, true}) {
    for (const bool deferred : {false, true}) {
      std::printf("self-close %s / %s\n", render ? "render" : "event",
                  deferred ? "deferred" : "direct");
      std::fflush(stdout);
      if (const int result = self_close(render, deferred)) return result;
    }
  }
  return 0;
}
