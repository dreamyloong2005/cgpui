#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::set_cursor(CursorShape cursor_shape) {
  cursor_shape_ = cursor_shape;
}

CursorShape WaylandWindow::cursor_shape() const {
  return cursor_shape_;
}

bool WaylandWindow::configured() const {
  return configured_;
}

void WaylandWindow::wakeup_requested() {
  callback_(WindowWakeupRequested{});
}

void WaylandWindow::focus_changed(bool focused) {
  callback_(WindowFocused{.focused = focused});
}

} // namespace cgpui
