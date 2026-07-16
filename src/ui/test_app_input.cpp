#include "test_app_internal.hpp"

#include "cgpui/ui/key_binding.hpp"

namespace cgpui {
namespace {

constexpr char test_app_action_name[] = "__cgpui_test_app_keystroke__";

KeyboardKey key_from_chord(const KeyBindingChord& chord) {
  return KeyboardKey{
      .key_code = chord.key_code,
      .action = chord.action,
      .modifiers = chord.modifiers};
}

} // namespace

ViewInputState TestAppWindow::input_state() const {
  return window().input_state();
}

bool TestAppWindow::dispatch_platform_event(PlatformEvent event) const {
  return state_->dispatch_event(runtime_id_, event);
}

void TestAppWindow::dispatch_keystroke(KeyboardKey key) const {
  (void)state_->dispatch_event(runtime_id_, PlatformEvent{key});
}

bool TestAppWindow::simulate_keystrokes(std::string_view keystrokes) const {
  const std::optional<KeyBinding> parsed =
      parse_key_binding(keystrokes, test_app_action_name);
  if (!parsed.has_value()) return false;
  if (parsed->sequence.empty()) {
    dispatch_keystroke(KeyboardKey{
        .key_code = parsed->key_code,
        .action = parsed->action,
        .modifiers = parsed->modifiers});
    return true;
  }
  for (const KeyBindingChord& chord : parsed->sequence) {
    dispatch_keystroke(key_from_chord(chord));
  }
  return true;
}

void TestAppWindow::dispatch_pointer_move(Point position) const {
  (void)state_->dispatch_event(runtime_id_, PlatformEvent{PointerMoved{position}});
}

void TestAppWindow::dispatch_pointer_button(
    MouseButton button,
    bool pressed,
    Point position) const {
  (void)state_->dispatch_event(
      runtime_id_, PlatformEvent{PointerButton{button, pressed, 1, position}});
}

void TestAppWindow::dispatch_pointer_scroll(Point delta, Point position) const {
  (void)state_->dispatch_event(
      runtime_id_, PlatformEvent{PointerScrolled{delta, position}});
}

void TestAppWindow::dispatch_window_activation(bool active) const {
  (void)state_->dispatch_event(runtime_id_, PlatformEvent{WindowActivated{active}});
}

void TestAppWindow::dispatch_window_focus(bool focused) const {
  (void)state_->dispatch_event(runtime_id_, PlatformEvent{WindowFocused{focused}});
}

void TestAppWindow::focus(ElementId element_id) const {
  state_->focus(runtime_id_, element_id);
}

void TestAppWindow::release_focus(ElementId element_id) const {
  state_->release_focus(runtime_id_, element_id);
}

} // namespace cgpui

namespace cgpui::detail {

bool TestAppState::dispatch_event(
    WindowRuntimeId runtime_id,
    const PlatformEvent& event) {
  const WindowRuntimeRecord* record = runtime.window_runtime_record(runtime_id);
  if (record == nullptr || !record->active || record->window == nullptr) {
    return false;
  }
  auto* window = dynamic_cast<TestPlatformWindow*>(record->window);
  if (window == nullptr) return false;
  window->dispatch_event(event);
  return true;
}

void TestAppState::focus(WindowRuntimeId runtime_id, ElementId element_id) {
  runtime.request_keyboard_focus_for_window(runtime_id, element_id);
}

void TestAppState::release_focus(
    WindowRuntimeId runtime_id,
    ElementId element_id) {
  runtime.release_keyboard_focus_for_window(runtime_id, element_id);
}

} // namespace cgpui::detail
