#include "wayland_application_internal.hpp"

#include <cmath>

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
    std::uint32_t serial,
    const WaylandCursorThemeApplyResult& result) {
  cursor_theme_status_ = result.theme_loaded
      ? WaylandCursorThemeLoadStatus::loaded
      : WaylandCursorThemeLoadStatus::unavailable;
  cursor_theme_state_.cursor_theme_status = cursor_theme_status_;
  cursor_theme_state_.image = WaylandCursorImageState{
      .shape = cursor_shape,
      .cursor_name = cursor_name_for_shape(cursor_shape),
      .status = result.image_ready ? WaylandCursorImageStatus::ready
                                   : WaylandCursorImageStatus::cursor_image_unavailable,
      .hotspot_x = result.hotspot_x,
      .hotspot_y = result.hotspot_y,
      .buffer_scale = result.buffer_scale,
      .unavailable_reason = result.unavailable_reason,
  };
  cursor_theme_state_.serial = serial;
  cursor_theme_state_.apply_count += 1;
}

void WaylandApplication::apply_cursor_for(WaylandWindow* window) {
  if (window == nullptr || pointer_ == nullptr) {
    return;
  }
  const CursorShape shape = wayland_window_cursor_shape(*window);
  const std::int32_t scale = static_cast<std::int32_t>(std::ceil(
      wayland_window_state(*window).scale.value));
  const WaylandCursorThemeApplyResult result = cursor_theme_ == nullptr
      ? WaylandCursorThemeApplyResult{
            .buffer_scale = std::max(scale, 1),
            .unavailable_reason = "cursor theme resources unavailable"}
      : apply_wayland_cursor_theme(
            *cursor_theme_, pointer_, pointer_enter_serial_,
            cursor_name_for_shape(shape), scale);
  record_cursor_theme_state(shape, pointer_enter_serial_, result);
  if (!result.applied) {
    wl_pointer_set_cursor(pointer_, pointer_enter_serial_, nullptr, 0, 0);
  }
  (void)wl_display_flush(display_);
}

void WaylandApplication::cursor_scale_changed(WaylandWindow& window) {
  if (pointer_window_ == &window) apply_cursor_for(&window);
}

} // namespace cgpui
