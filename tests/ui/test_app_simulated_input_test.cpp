#include "cgpui/ui/test_app.hpp"

#include <memory>
#include <variant>

namespace {

bool equal(cgpui::Point lhs, cgpui::Point rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

class InputView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext&) override {
    if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event)) {
      key_count += 1;
      last_key = *key;
    } else if (const auto* moved = std::get_if<cgpui::PointerMoved>(&event)) {
      move_count += 1;
      last_position = moved->position;
    } else if (const auto* button = std::get_if<cgpui::PointerButton>(&event)) {
      button_count += 1;
      last_button = *button;
    } else if (const auto* scroll =
                   std::get_if<cgpui::PointerScrolled>(&event)) {
      scroll_count += 1;
      last_scroll = *scroll;
    } else if (const auto* focused =
                   std::get_if<cgpui::WindowFocused>(&event)) {
      focus_count += 1;
      last_focused = focused->focused;
    }
    return cgpui::EventResult::unhandled();
  }

  int key_count = 0;
  int move_count = 0;
  int button_count = 0;
  int scroll_count = 0;
  int focus_count = 0;
  bool last_focused = false;
  cgpui::KeyboardKey last_key{};
  cgpui::Point last_position{};
  cgpui::PointerButton last_button{};
  cgpui::PointerScrolled last_scroll{};
};

} // namespace

int main() {
  cgpui::TestApp app;
  cgpui::TestAppWindow window = app.open_window(
      cgpui::WindowOptions{}.size(320.0F, 180.0F),
      std::make_unique<InputView>());
  InputView* view = window.root_view_as<InputView>();
  if (view == nullptr) return 1;

  window.dispatch_keystroke(cgpui::KeyboardKey{.key_code = 'I'});
  if (!window.simulate_keystrokes("ctrl-k ctrl-s") ||
      window.simulate_keystrokes("ctrl-unknown") || view->key_count != 3 ||
      view->last_key.key_code != 'S' || !view->last_key.modifiers.control) {
    return 2;
  }

  window.dispatch_pointer_move({5.0F, 7.0F});
  window.dispatch_pointer_button(
      cgpui::MouseButton::left, true, {8.0F, 9.0F});
  window.dispatch_pointer_button(
      cgpui::MouseButton::left, false, {8.0F, 9.0F});
  window.dispatch_pointer_scroll({0.0F, -3.0F}, {11.0F, 13.0F});
  if (view->move_count != 1 || view->button_count != 2 ||
      view->scroll_count != 1 || view->last_button.pressed ||
      !equal(view->last_scroll.delta, {0.0F, -3.0F}) ||
      !equal(window.input_state().pointer_position, {11.0F, 13.0F})) {
    return 3;
  }

  cgpui::TestAppWindow second = app.open_window(
      cgpui::WindowOptions{}.title("Second Input Window"),
      std::make_unique<InputView>());
  second.dispatch_pointer_move({21.0F, 34.0F});
  if (!equal(window.input_state().pointer_position, {11.0F, 13.0F}) ||
      !equal(second.input_state().pointer_position, {21.0F, 34.0F})) {
    return 6;
  }

  window.dispatch_window_activation(true);
  window.dispatch_window_focus(true);
  window.focus(cgpui::ElementId{88});
  const cgpui::ViewInputState focused = window.input_state();
  if (!focused.focused || !focused.keyboard_focused ||
      focused.keyboard_focus_element_owner != cgpui::ElementId{88} ||
      view->focus_count != 1 || !view->last_focused) {
    return 4;
  }
  window.release_focus(cgpui::ElementId{88});
  window.dispatch_window_focus(false);
  window.dispatch_window_activation(false);
  const cgpui::ViewInputState released = window.input_state();
  if (released.focused || released.keyboard_focused ||
      released.keyboard_focus_element_owner.has_value() ||
      view->focus_count != 2 || view->last_focused) {
    return 5;
  }
  return 0;
}
