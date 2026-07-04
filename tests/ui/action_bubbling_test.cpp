#include "window_runtime_test_support.hpp"

#include <variant>

namespace {

RuntimeFixture* bubbling_fixture = nullptr;

void dispatch_action_bubbling_sequence() {
  auto& callback = bubbling_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 66,
      .action = cgpui::KeyAction::pressed});
}

class ActionBubblingView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    context.focus(focused_element_id);
    register_unhandled_bubble_chain(context);
    register_disabled_bubble_chain(context);
    register_cancelled_stop_chain(context);

    unhandled_bubble_result =
        context.dispatch_action("bubble.unhandled.to.window");
    disabled_bubble_result =
        context.dispatch_action("bubble.disabled.to.view");
    cancelled_stop_result =
        context.dispatch_action("bubble.cancelled.stop");

    return cgpui::EventResult::consumed_event();
  }

  void register_unhandled_bubble_chain(
      const cgpui::WindowRuntimeContext& context) {
    context.register_focused_element_action(
        focused_element_id,
        "bubble.unhandled.to.window",
        [this](const cgpui::WindowRuntimeContext&) {
          focused_unhandled_count += 1;
          return cgpui::EventResult::unhandled();
        });
    context.register_view_action(
        "bubble.unhandled.to.window",
        [this](const cgpui::WindowRuntimeContext&) {
          view_unhandled_count += 1;
          return cgpui::EventResult::unhandled();
        });
    context.register_window_action(
        "bubble.unhandled.to.window",
        [this](const cgpui::WindowRuntimeContext&) {
          window_consumed_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_app_action(
        "bubble.unhandled.to.window",
        [this](const cgpui::WindowRuntimeContext&) {
          app_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_disabled_bubble_chain(
      const cgpui::WindowRuntimeContext& context) {
    context.register_focused_element_action(
        focused_element_id,
        "bubble.disabled.to.view",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_focused_count += 1;
          return cgpui::EventResult::consumed_event();
        },
        cgpui::ActionRegistrationOptions{.enabled = false});
    context.register_view_action(
        "bubble.disabled.to.view",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_view_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "bubble.disabled.to.view",
        [this](const cgpui::WindowRuntimeContext&) {
          disabled_window_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  void register_cancelled_stop_chain(
      const cgpui::WindowRuntimeContext& context) {
    context.register_focused_element_action(
        focused_element_id,
        "bubble.cancelled.stop",
        [this](const cgpui::WindowRuntimeContext&) {
          cancelled_focused_count += 1;
          return cgpui::EventResult::cancelled_event();
        });
    context.register_view_action(
        "bubble.cancelled.stop",
        [this](const cgpui::WindowRuntimeContext&) {
          cancelled_view_shadow_count += 1;
          return cgpui::EventResult::consumed_event();
        });
  }

  cgpui::ElementId focused_element_id{88};
  int focused_unhandled_count = 0;
  int view_unhandled_count = 0;
  int window_consumed_count = 0;
  int app_shadow_count = 0;
  int disabled_focused_count = 0;
  int disabled_view_count = 0;
  int disabled_window_shadow_count = 0;
  int cancelled_focused_count = 0;
  int cancelled_view_shadow_count = 0;
  cgpui::ActionDispatchResult unhandled_bubble_result{};
  cgpui::ActionDispatchResult disabled_bubble_result{};
  cgpui::ActionDispatchResult cancelled_stop_result{};
};

int test_action_dispatch_bubbles_past_unhandled_and_disabled_scopes() {
  RuntimeFixture fixture;
  ActionBubblingView view;
  bubbling_fixture = &fixture;
  fixture.app.on_run = &dispatch_action_bubbling_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  bubbling_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (view.focused_unhandled_count != 1 ||
      view.view_unhandled_count != 1 ||
      view.window_consumed_count != 1 ||
      view.app_shadow_count != 0) {
    return 2;
  }
  if (!view.unhandled_bubble_result.handled ||
      !view.unhandled_bubble_result.scope.has_value() ||
      *view.unhandled_bubble_result.scope != cgpui::ActionScope::window ||
      !view.unhandled_bubble_result.result.consumed ||
      view.unhandled_bubble_result.result.cancelled) {
    return 3;
  }
  if (view.disabled_focused_count != 0 ||
      view.disabled_view_count != 1 ||
      view.disabled_window_shadow_count != 0) {
    return 4;
  }
  if (!view.disabled_bubble_result.handled ||
      !view.disabled_bubble_result.scope.has_value() ||
      *view.disabled_bubble_result.scope != cgpui::ActionScope::view ||
      !view.disabled_bubble_result.view_id.has_value() ||
      *view.disabled_bubble_result.view_id != cgpui::ViewId{1}) {
    return 5;
  }
  if (view.cancelled_focused_count != 1 ||
      view.cancelled_view_shadow_count != 0) {
    return 6;
  }
  if (!view.cancelled_stop_result.handled ||
      !view.cancelled_stop_result.scope.has_value() ||
      *view.cancelled_stop_result.scope != cgpui::ActionScope::focused_element ||
      !view.cancelled_stop_result.result.consumed ||
      !view.cancelled_stop_result.result.cancelled) {
    return 7;
  }

  return 0;
}

} // namespace

int main() {
  return test_action_dispatch_bubbles_past_unhandled_and_disabled_scopes();
}
