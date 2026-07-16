#include "x11_window_internal.hpp"

namespace cgpui {
namespace {

template <typename Event>
Point pointer_position(const Event& event) {
  return Point{
      static_cast<float>(event.event_x),
      static_cast<float>(event.event_y)};
}

std::optional<MouseButton> mouse_button(std::uint8_t detail) {
  switch (detail) {
    case 1: return MouseButton::left;
    case 2: return MouseButton::middle;
    case 3: return MouseButton::right;
    case 8: return MouseButton::back;
    case 9: return MouseButton::forward;
    default: return detail >= 10 ? std::optional{MouseButton::other} : std::nullopt;
  }
}

std::optional<Point> scroll_delta(std::uint8_t detail) {
  switch (detail) {
    case 4: return Point{0.0F, 1.0F};
    case 5: return Point{0.0F, -1.0F};
    case 6: return Point{-1.0F, 0.0F};
    case 7: return Point{1.0F, 0.0F};
    default: return std::nullopt;
  }
}

}  // namespace

void X11Window::handle_pointer_event(const xcb_generic_event_t& event) {
  const std::uint8_t type = event.response_type & 0x7f;
  if (type == XCB_MOTION_NOTIFY) {
    const auto& motion = reinterpret_cast<const xcb_motion_notify_event_t&>(event);
    callback_(PointerMoved{.position = pointer_position(motion)});
    return;
  }
  if (type == XCB_LEAVE_NOTIFY) {
    const auto& leave = reinterpret_cast<const xcb_leave_notify_event_t&>(event);
    callback_(PointerExited{.position = pointer_position(leave)});
    return;
  }
  if (type != XCB_BUTTON_PRESS && type != XCB_BUTTON_RELEASE) return;
  const auto& button = reinterpret_cast<const xcb_button_press_event_t&>(event);
  if (const auto scroll = scroll_delta(button.detail);
      scroll && type == XCB_BUTTON_PRESS) {
    callback_(PointerScrolled{
        .delta = *scroll,
        .position = pointer_position(button),
        .precise = false,
    });
    return;
  }
  if (const auto mapped = mouse_button(button.detail)) {
    callback_(PointerButton{
        .button = *mapped,
        .pressed = type == XCB_BUTTON_PRESS,
        .click_count = 1,
        .position = pointer_position(button),
    });
  }
}

}  // namespace cgpui
