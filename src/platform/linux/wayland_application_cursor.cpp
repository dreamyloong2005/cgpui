#include "wayland_application_internal.hpp"

namespace cgpui {

void WaylandApplication::set_window_cursor(
    WaylandWindow& window,
    CursorShape cursor_shape) {
  wayland_window_set_cursor(window, cursor_shape);
  if (pointer_window_ == &window) {
    apply_cursor_for(&window);
  }
}

void WaylandApplication::record_cursor_theme_state(
    CursorShape cursor_shape,
    std::uint32_t serial) {
  cursor_theme_state_.cursor_theme_status = cursor_theme_status_;
  cursor_theme_state_.image = WaylandCursorImageState{
      .shape = cursor_shape,
      .cursor_name = cursor_name_for_shape(cursor_shape),
      .status = WaylandCursorImageStatus::cursor_image_unavailable,
      .hotspot_x = 0,
      .hotspot_y = 0,
      .unavailable_reason = "cursor theme image unavailable",
  };
  cursor_theme_state_.serial = serial;
  cursor_theme_state_.apply_count += 1;
}

void WaylandApplication::apply_cursor_for(WaylandWindow* window) {
  if (window == nullptr || pointer_ == nullptr) {
    return;
  }
  record_cursor_theme_state(
      wayland_window_cursor_shape(*window),
      pointer_enter_serial_);
  wl_pointer_set_cursor(pointer_, pointer_enter_serial_, nullptr, 0, 0);
  (void)wl_display_flush(display_);
}

} // namespace cgpui
