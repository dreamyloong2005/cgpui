#include "runtime_window_lifecycle_test_support.hpp"

namespace {
using namespace runtime_window_test;

void request(const cgpui::WindowRuntimeContext& context, int operation) {
  switch (operation) {
    case 0: context.request_render(); break;
    case 1: context.request_layout(); break;
    case 2: context.request_paint(); break;
    case 3: context.current_window().request_render(); break;
    case 4: context.current_window().request_layout(); break;
    case 5: context.window_context().request_paint(); break;
  }
}

int isolated_redraw(int operation) {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, root, [&](const auto&) -> cgpui::Result<cgpui::Renderer*> {
    renderers.push_back(std::make_unique<Renderer>(app.last_created));
    return renderers.back().get();
  });
  int failure = 0;
  app.on_run = [&] {
    auto view = std::make_unique<View>();
    view->on_event = [&](const auto&, const auto& context) {
      request(context, operation);
      request(context, operation);
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime.open_window(cgpui::WindowOptions{}.title("target"), std::move(view));
    const auto target = app.children.back();
    (void)runtime.open_window(cgpui::WindowOptions{}.title("unrelated"), std::make_unique<View>());
    const auto unrelated = app.children.back();
    target->emit(key());
    if (target->redraws != 1 || app.root->redraws != 0 || unrelated->redraws != 0) failure = 1;
    const auto* record = runtime.window_runtime_record(opened.runtime_id);
    if (record == nullptr || !record->redraw_scheduled) failure = 2;
    target->emit(cgpui::WindowRedrawRequested{});
    if (target->presents != 1) failure = 3;
    target->emit(key());
    if (target->redraws != 2 || app.root->redraws != 0 || unrelated->redraws != 0) failure = 4;
  };
  const int result = runtime.run({}, {.request_initial_redraw = false});
  return result != 0 ? 5 : failure;
}

int redraw_requested_during_frame_survives() {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, root, [&](const auto&) -> cgpui::Result<cgpui::Renderer*> {
    renderers.push_back(std::make_unique<Renderer>(app.last_created));
    return renderers.back().get();
  });
  int failure = 0;
  app.on_run = [&] {
    bool first_frame = true;
    auto view = std::make_unique<View>();
    view->on_render = [&](const auto& context) {
      if (first_frame) {
        first_frame = false;
        context.request_render();
        context.request_render();
      }
    };
    const auto opened = runtime.open_window(cgpui::WindowOptions{}.title("next frame"), std::move(view));
    const auto native = app.children.back();
    native->emit(cgpui::WindowRedrawRequested{});
    app.wake();
    const auto* record = runtime.window_runtime_record(opened.runtime_id);
    if (native->presents != 1 || native->redraws != 1 || record == nullptr ||
        !record->redraw_scheduled || app.root->redraws != 0) failure = 61;
    native->emit(cgpui::WindowRedrawRequested{});
    app.wake();
    if (native->presents != 2 || native->redraws != 1 || app.root->redraws != 0) failure = 62;
    cgpui::Window handle(runtime, opened.runtime_id);
    native->emit(cgpui::WindowCloseRequested{});
    app.wake();
    handle.request_render();
    handle.request_layout();
    handle.request_paint();
    if (native->redraws != 1 || app.root->redraws != 0) failure = 63;
  };
  const int result = runtime.run({}, {.request_initial_redraw = false});
  return result != 0 ? 64 : failure;
}
} // namespace

int main() {
  for (int operation = 0; operation < 6; ++operation) {
    if (const int result = isolated_redraw(operation)) return operation * 10 + result;
  }
  return redraw_requested_during_frame_survives();
}
