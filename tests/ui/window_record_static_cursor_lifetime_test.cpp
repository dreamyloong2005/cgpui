#include "window_record_static_cursor_test_support.hpp"

#include <array>
#include <iostream>

namespace {
using namespace runtime_window_test;
using namespace runtime_window_cursor_test;

struct CursorProbe {
  cgpui::WindowRuntime* runtime = nullptr;
  cgpui::WindowRuntimeId id;
  cgpui::EventRoute route;
  bool view_destroyed = false;
  bool callback_returned_alive = false;
  int cursor_callbacks = 0;
  int events = 0;
  int close_successes = 0;
  int failure = 0;
};

class ReplacingStaticView final : public cgpui::View {
 public:
  explicit ReplacingStaticView(CursorProbe& probe) : probe_(probe) {}
  ~ReplacingStaticView() override { probe_.view_destroyed = true; }
  bool supports_static_render() const override { return !dynamic; }
  cgpui::AnyElement render(cgpui::ViewContext&) override {
    return cgpui::into_element(cgpui::div().size(120, 80));
  }
  bool dynamic = false;
  bool text_cursor_on_root = false;
  cgpui::StaticElementTreeView render_static(cgpui::ViewContext& context) override {
    context.set_element_cursor(cgpui::ElementId{2}, cgpui::CursorShape::pointing_hand);
    if (text_cursor_on_root) context.set_element_cursor(cgpui::ElementId{1}, cgpui::CursorShape::text);
    const cgpui::StaticElementTreeView tree{nodes_, children_, cgpui::ElementId{1}};
    if (!tree.valid()) probe_.failure = 1;
    return tree;
  }
  void paint(cgpui::PaintList&, cgpui::Size) override {}
  cgpui::EventResult handle_event(const cgpui::PlatformEvent&,
                                 const cgpui::WindowRuntimeContext& context) override {
    ++probe_.events;
    if (context.event_route) probe_.route = *context.event_route;
    return cgpui::EventResult::unhandled();
  }
  void replace_storage(bool outside = true) {
    // Keep the old allocation alive so stale-span routing fails semantically,
    // rather than depending on an allocator-specific use-after-free crash.
    old_nodes_ = std::move(nodes_);
    nodes_ = old_nodes_;
    nodes_[1].bounds = outside ? cgpui::Rect{{90, 60}, {10, 10}}
                               : cgpui::Rect{{10, 10}, {60, 40}};
    if (nodes_.data() == old_nodes_.data()) probe_.failure = 2;
  }

 private:
  CursorProbe& probe_;
  std::vector<cgpui::StaticElementNode> nodes_{
      cgpui::StaticElementNode{.id = {1}, .children = {0, 1},
                               .bounds = {{0, 0}, {120, 80}}},
      cgpui::StaticElementNode{.id = {2}, .parent_id = {1},
                               .bounds = {{10, 10}, {60, 40}}, .focusable = true}};
  std::vector<cgpui::StaticElementNode> old_nodes_;
  std::array<cgpui::ElementId, 1> children_{cgpui::ElementId{2}};
};

int cursor_callback_reenters_static_routing(int action) {
  // 0: first callback closes; 1/2: replaces static storage/switches dynamic;
  // 3/4/5: second callback closes/switches dynamic/selects a third cursor;
  // 6: every callback redraws an oscillating hit, then next input recovers.
  const bool close_window = action == 0 || action == 3;
  CursorApplication app;
  View root;
  CursorProbe probe;
  app.base.on_run = [&] {
    auto view = std::make_unique<ReplacingStaticView>(probe);
    auto* view_pointer = view.get();
    const auto opened = probe.runtime->try_open_window(popup_options(), std::move(view));
    if (!opened) { probe.failure = 3; return; }
    probe.id = opened->runtime_id;
    const auto cursor = app.children.back();
    const auto native = cursor->native;
    native->emit(cgpui::WindowRedrawRequested{});
    cursor->on_cursor = [&, native](cgpui::CursorShape shape) {
      const int count = ++probe.cursor_callbacks;
      if (count > 3) { probe.failure = 4; return; }
      const auto expected = count == 1 ? cgpui::CursorShape::pointing_hand
          : count == 2 ? cgpui::CursorShape::default_arrow : cgpui::CursorShape::text;
      if (shape != expected) { probe.failure = 5; return; }
      if (action == 0 || (action == 3 && count == 2)) {
        if (cgpui::close_popup(*probe.runtime, probe.id)) ++probe.close_successes;
      } else if (count == 1 || (count == 2 && action >= 4)) {
        if (action == 2 || (action == 4 && count == 2)) view_pointer->dynamic = true;
        else if (action == 5 && count == 2) view_pointer->text_cursor_on_root = true;
        else view_pointer->replace_storage(count == 1);
        native->emit(cgpui::WindowRedrawRequested{});
      }
      probe.callback_returned_alive = !probe.view_destroyed && !native->destroyed;
    };
    native->emit(cgpui::PointerMoved{.position = {20, 20}});
    cursor->on_cursor = {};
    if (probe.failure != 0) return;
    const int expected_callbacks = action == 0 ? 1 : action == 5 ? 3 : 2;
    if (probe.cursor_callbacks != expected_callbacks ||
        !probe.callback_returned_alive ||
        native->destroyed || native->destroyed_during_callback) {
      probe.failure = 6;
      return;
    }
    const auto* record = probe.runtime->window_runtime_record(probe.id);
    if (close_window) {
      if (probe.close_successes != 1 || probe.events != 0 || !probe.view_destroyed ||
          record == nullptr || record->active ||
          record->input.cursor_shape != cgpui::CursorShape::default_arrow ||
          record->input.hovered_element_id || record->input.active_element_id ||
          record->input.keyboard_focus_element_owner || record->input.pointer_down_element_id)
        probe.failure = 7;
    } else {
      const cgpui::ElementId target{action == 6 ? 2U : 1U};
      const auto ancestry = action == 6 ? std::vector<cgpui::ElementId>{{2}, {1}}
                                         : std::vector<cgpui::ElementId>{{1}};
      const auto expected_cursor = action == 5 ? cgpui::CursorShape::text : cgpui::CursorShape::default_arrow;
      if (probe.view_destroyed || native->presents != (action >= 4 ? 3 : 2) || probe.events != 1 ||
          record == nullptr || !record->active ||
          probe.route.target_element_id != target || probe.route.element_ancestry != ancestry ||
          record->input.hovered_element_id != target ||
          record->input.cursor_shape != expected_cursor || cursor->cursor != expected_cursor ||
          record->input.active_element_id || record->input.keyboard_focus_element_owner ||
          record->input.pointer_down_element_id) probe.failure = 8;
      if (action == 6 && probe.failure == 0) {
        // The callback had no fixed point: route latest state, don't invent an
        // unsubmitted cursor. Once it stops mutating, the next input converges.
        native->emit(cgpui::PointerMoved{.position = {20, 20}});
        if (probe.events != 2 || probe.cursor_callbacks != 2 ||
            record->input.cursor_shape != cgpui::CursorShape::pointing_hand ||
            cursor->cursor != cgpui::CursorShape::pointing_hand ||
            probe.route.target_element_id != target) probe.failure = 12;
      }
      if (!cgpui::close_popup(*probe.runtime, probe.id)) probe.failure = 9;
    }
    app.base.wake();
    app.base.wake();
    if (!probe.view_destroyed || !native->destroyed || native->destroyed_during_callback ||
        native->renderer_destroyed_after_window || native->live_renderers != 0 ||
        probe.runtime->window_runtime_record(probe.id) != nullptr) probe.failure = 10;
  };
  cgpui::AppRunnerOptions options;
  options.runtime.request_initial_redraw = false;
  options.setup = [&](auto& runtime) { probe.runtime = &runtime; };
  const int result = cgpui::run_app(app, root, [&](const auto&)
      -> cgpui::Result<std::unique_ptr<cgpui::Renderer>> {
    return std::make_unique<Renderer>(app.base.last_created);
  }, options);
  return result != 0 ? 11 : probe.failure;
}
} // namespace

int main() {
  int failure = 0;
  for (const int action : {0, 1, 2, 3, 4, 5, 6}) {
    if (const int result = cursor_callback_reenters_static_routing(action); result != 0) {
      std::cerr << "static cursor " << action
                << " failure: " << result << '\n';
      if (failure == 0) failure = result;
    }
  }
  return failure;
}
