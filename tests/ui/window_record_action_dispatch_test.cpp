// An element action fired in an additional window dispatches through that
// window: its focused element, its view, and handlers run with its context.
#include "cgpui/ui/test_app.hpp"
#include "cgpui/ui/ui.hpp"

#include <iostream>
#include <memory>
#include <optional>
#include <string_view>

namespace {

class ActionView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    context.runtime.register_view_action(
        context.view_id, "demo.act",
        [this](const cgpui::WindowRuntimeContext& handler_context) {
          handled_count += 1;
          handled_in = handler_context.window_runtime_id;
          return cgpui::EventResult::consumed_event();
        });
    return cgpui::button("demo.act")
        .child(cgpui::div().size(80.0F, 30.0F))
        .build();
  }

  int handled_count = 0;
  std::optional<cgpui::WindowRuntimeId> handled_in;
};

int fail(int code, std::string_view message) {
  std::cerr << message << '\n';
  return code;
}

} // namespace

int main() {
  cgpui::TestApp app;
  cgpui::TestAppWindow first = app.open_window(
      cgpui::WindowOptions{}.size(200.0F, 100.0F),
      std::make_unique<ActionView>());
  cgpui::TestAppWindow second = app.open_window(
      cgpui::WindowOptions{}.size(200.0F, 100.0F),
      std::make_unique<ActionView>());
  first.draw_frame();
  second.draw_frame();

  second.dispatch_pointer_button(cgpui::MouseButton::left, true, {10, 10});
  second.dispatch_pointer_button(cgpui::MouseButton::left, false, {10, 10});

  auto* first_view = first.root_view_as<ActionView>();
  auto* second_view = second.root_view_as<ActionView>();
  if (first_view->handled_count != 0) {
    return fail(1, "the other window's view must not handle the action");
  }
  if (second_view->handled_count != 1) {
    return fail(2, "the clicked window's view must handle the action");
  }
  if (second_view->handled_in != second.runtime_id()) {
    return fail(3, "the handler must run with the clicked window's context");
  }
  return 0;
}
