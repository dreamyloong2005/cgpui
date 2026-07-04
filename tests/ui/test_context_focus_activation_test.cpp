#include "window_runtime_test_support.hpp"

#include <optional>
#include <vector>
#include <variant>

namespace {

RuntimeFixture* test_context_focus_fixture = nullptr;

void dispatch_initial_focus_simulation_event() {
  test_context_focus_fixture->window.callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
}

class TestContextFocusView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
        key != nullptr && key->key_code == 'I' && !queued_simulation) {
      queued_simulation = true;
      context.defer([this](const cgpui::WindowRuntimeContext& deferred) {
        const cgpui::TestContextCapability test_context =
            deferred.test_context();
        test_context.dispatch_window_activation(true);
        test_context.dispatch_window_focus(true);
        test_context.focus(focused_element_id);
        input_after_focus = test_context.input_state();
        test_context.dispatch_keystroke(cgpui::KeyboardKey{
            .key_code = 'F',
            .action = cgpui::KeyAction::pressed});
        test_context.release_focus(focused_element_id);
        input_after_release = test_context.input_state();
        test_context.dispatch_window_focus(false);
        test_context.dispatch_window_activation(false);
      });
      return cgpui::EventResult::consumed_event();
    }

    if (const auto* focused = std::get_if<cgpui::WindowFocused>(&event);
        focused != nullptr) {
      window_focus_count += 1;
      window_focus_values.push_back(focused->focused);
      window_focus_context_values.push_back(
          context.test_context().input_state().focused);
    } else if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
               key != nullptr && key->key_code == 'F') {
      focused_key_count += 1;
      focused_key_route = context.current_event_route();
      focused_key_input = context.test_context().input_state();
    }

    return cgpui::EventResult::unhandled();
  }

  bool queued_simulation = false;
  cgpui::ElementId focused_element_id{88};
  int window_focus_count = 0;
  int focused_key_count = 0;
  std::vector<bool> window_focus_values;
  std::vector<bool> window_focus_context_values;
  cgpui::ViewInputState input_after_focus{};
  cgpui::ViewInputState input_after_release{};
  cgpui::ViewInputState focused_key_input{};
  std::optional<cgpui::EventRoute> focused_key_route;
};

int test_test_context_simulates_focus_and_window_activation() {
  RuntimeFixture fixture;
  TestContextFocusView view;
  test_context_focus_fixture = &fixture;
  fixture.app.on_run = &dispatch_initial_focus_simulation_event;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  std::vector<cgpui::EventKind> event_kinds;
  std::vector<bool> focused_states;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        event_kinds.push_back(record.event_kind);
        focused_states.push_back(context.test_context().input_state().focused);
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  test_context_focus_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  const std::vector<cgpui::EventKind> expected_kinds = {
      cgpui::EventKind::keyboard_key,
      cgpui::EventKind::window_activated,
      cgpui::EventKind::window_focused,
      cgpui::EventKind::keyboard_key,
      cgpui::EventKind::window_focused,
      cgpui::EventKind::window_activated};
  if (event_kinds != expected_kinds || focused_states.size() != 6) {
    return 2;
  }
  if (!focused_states[1] || !focused_states[2] || !focused_states[3] ||
      focused_states[4] || focused_states[5]) {
    return 3;
  }
  if (view.window_focus_count != 2 ||
      view.window_focus_values != std::vector<bool>{true, false} ||
      view.window_focus_context_values != std::vector<bool>{true, false}) {
    return 4;
  }
  if (!view.input_after_focus.keyboard_focused ||
      !view.input_after_focus.keyboard_focus_element_owner.has_value() ||
      *view.input_after_focus.keyboard_focus_element_owner !=
          view.focused_element_id) {
    return 5;
  }
  if (view.input_after_release.keyboard_focused ||
      view.input_after_release.keyboard_focus_element_owner.has_value()) {
    return 6;
  }
  if (view.focused_key_count != 1 || !view.focused_key_route.has_value() ||
      !view.focused_key_route->target_element_id.has_value() ||
      *view.focused_key_route->target_element_id != view.focused_element_id) {
    return 7;
  }
  if (!view.focused_key_input.keyboard_focused ||
      !view.focused_key_input.keyboard_focus_element_owner.has_value() ||
      *view.focused_key_input.keyboard_focus_element_owner !=
          view.focused_element_id) {
    return 8;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_simulates_focus_and_window_activation();
}
