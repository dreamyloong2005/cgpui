#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::pointer_moved(Point position) {
  callback_(PointerMoved{.position = position});
}

void WaylandWindow::pointer_exited(Point position) {
  callback_(PointerExited{.position = position});
}

void WaylandWindow::pointer_button(
    MouseButton button,
    bool pressed,
    Point position) {
  callback_(PointerButton{
      .button = button,
      .pressed = pressed,
      .position = position,
  });
}

void WaylandWindow::pointer_scrolled(Point delta, Point position) {
  callback_(PointerScrolled{.delta = delta, .position = position});
}

void WaylandWindow::keyboard_key(
    std::uint32_t key,
    KeyAction action,
    KeyboardModifiers modifiers) {
  callback_(KeyboardKey{
      .key_code = key,
      .action = action,
      .modifiers = modifiers});
}

} // namespace cgpui
