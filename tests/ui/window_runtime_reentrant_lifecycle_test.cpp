#include "runtime_window_lifecycle_test_support.hpp"

#include <cstdio>

namespace {
using namespace runtime_window_test;

int mutating_callback(bool render, bool erase_other) {
  Application app;
  View root;
  std::vector<std::unique_ptr<Renderer>> renderers;
  cgpui::WindowRuntime runtime(app, root, [&](const cgpui::RenderSurfaceDescriptor&)
      -> cgpui::Result<cgpui::Renderer*> {
    renderers.push_back(std::make_unique<Renderer>(app.last_created));
    return renderers.back().get();
  });
  bool callback_returned = false;
  bool event_recorded = false;
  int failure = 0;
  app.on_run = [&] {
    cgpui::WindowRuntimeId victim;
    if (erase_other) victim = runtime.open_window(popup_options(), std::make_unique<View>()).runtime_id;
    auto view = std::make_unique<View>();
    auto mutate = [&](const cgpui::WindowRuntimeContext& context) {
      if (erase_other) {
        if (!cgpui::close_popup(context.runtime, victim)) failure = 1;
      } else {
        for (int i = 0; i < 64; ++i) {
          (void)context.runtime.open_window(cgpui::WindowOptions{}.title("additional"));
        }
      }
      callback_returned = true;
    };
    if (render) view->on_render = mutate;
    else view->on_event = [&](const auto&, const auto& context) {
      mutate(context);
      return cgpui::EventResult::consumed_event();
    };
    const auto opened = runtime.open_window(cgpui::WindowOptions{}.title("target"), std::move(view));
    const auto native = app.children.back();
    runtime.set_after_event_callback([&](const auto& context, const auto& dispatch) {
      if (context.window_runtime_id == opened.runtime_id) {
        event_recorded = dispatch.view_id == opened.root_view_id && dispatch.result.consumed;
      }
    });
    native->emit(render ? cgpui::PlatformEvent{cgpui::WindowRedrawRequested{}}
                        : cgpui::PlatformEvent{key()});
    const auto* record = runtime.window_runtime_record(opened.runtime_id);
    if (!callback_returned || record == nullptr || !record->active) failure = 2;
    else if (render ? native->presents != 1
                    : !event_recorded || !record->last_event_dispatch.has_value()) failure = 3;
  };
  const int result = runtime.run({}, {.request_initial_redraw = false});
  return result != 0 ? 4 : failure;
}
} // namespace

int main() {
  for (const bool render : {false, true}) {
    for (const bool erase_other : {false, true}) {
      std::printf("reentrant %s / %s\n", render ? "render" : "event",
                  erase_other ? "erase-other" : "growth");
      std::fflush(stdout);
      if (const int result = mutating_callback(render, erase_other)) return result;
    }
  }
  return 0;
}
