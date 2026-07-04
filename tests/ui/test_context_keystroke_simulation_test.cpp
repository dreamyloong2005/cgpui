#include "window_runtime_test_support.hpp"

#include <optional>
#include <variant>

namespace {

RuntimeFixture* test_context_key_fixture = nullptr;

void dispatch_initial_keystroke() {
  auto& callback = test_context_key_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 'I',
      .action = cgpui::KeyAction::pressed});
}

class TestContextKeystrokeView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr) {
      return cgpui::EventResult::unhandled();
    }

    key_event_count += 1;
    if (key->key_code != 'I') {
      return cgpui::EventResult::unhandled();
    }

    context.register_window_action(
        "test.keystroke.single",
        [this](const cgpui::WindowRuntimeContext&) {
          single_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    context.register_window_action(
        "test.keystroke.sequence",
        [this](const cgpui::WindowRuntimeContext&) {
          sequence_count += 1;
          return cgpui::EventResult::consumed_event();
        });
    bound_single = context.bind_key(
        "ctrl-j",
        "test.keystroke.single",
        cgpui::KeyBindingContext::window());
    bound_sequence = context.bind_key(
        "ctrl-k ctrl-s",
        "test.keystroke.sequence",
        cgpui::KeyBindingContext::window());

    const cgpui::TestContextCapability test_context =
        context.test_context();
    parsed_single = test_context.simulate_keystrokes("ctrl-j");
    parsed_sequence = test_context.simulate_keystrokes("ctrl-k ctrl-s");
    parsed_invalid = test_context.simulate_keystrokes("ctrl-unknown");
    test_context.dispatch_keystroke(cgpui::KeyboardKey{
        .key_code = 'J',
        .action = cgpui::KeyAction::pressed,
        .modifiers = {.control = true}});
    last_dispatch = test_context.last_action_dispatch();
    return cgpui::EventResult::consumed_event();
  }

  int key_event_count = 0;
  int single_count = 0;
  int sequence_count = 0;
  bool parsed_single = false;
  bool parsed_sequence = false;
  bool parsed_invalid = true;
  bool bound_single = false;
  bool bound_sequence = false;
  std::optional<cgpui::ActionDispatchResult> last_dispatch;
};

int test_test_context_simulates_keystrokes_through_runtime_dispatch() {
  RuntimeFixture fixture;
  TestContextKeystrokeView view;
  test_context_key_fixture = &fixture;
  fixture.app.on_run = &dispatch_initial_keystroke;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result = runtime.run(
      cgpui::WindowDescriptor{},
      cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  test_context_key_fixture = nullptr;

  if (result != 0) {
    return 1;
  }
  if (!view.parsed_single || !view.parsed_sequence || view.parsed_invalid) {
    return 2;
  }
  if (!view.bound_single || !view.bound_sequence) {
    return 7;
  }
  if (view.single_count != 2 || view.sequence_count != 1) {
    return 3;
  }
  if (view.key_event_count != 5) {
    return 4;
  }
  if (!view.last_dispatch.has_value() ||
      view.last_dispatch->name != "test.keystroke.single" ||
      !view.last_dispatch->handled) {
    return 5;
  }
  const std::optional<cgpui::ActionDispatchResult> runtime_dispatch =
      runtime.last_action_dispatch();
  if (!runtime_dispatch.has_value() ||
      runtime_dispatch->name != "test.keystroke.single") {
    return 6;
  }

  return 0;
}

} // namespace

int main() {
  return test_test_context_simulates_keystrokes_through_runtime_dispatch();
}
