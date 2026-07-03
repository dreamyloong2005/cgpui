#include "wayland_text_input_internal.hpp"

namespace cgpui {

void WaylandTextInput::apply_placement(WaylandWindow& window) {
  const auto placement = wayland_window_state(window).ime_text_input_placement;
  if (placement.has_value()) {
    const std::int32_t cursor =
        static_cast<std::int32_t>(placement->byte_offset);
    wayland_window_text_input_surrounding_text(window, {}, cursor, cursor);
    wayland_window_text_input_content_type(window, 0, 0);
  }
  if (text_input_ == nullptr) {
    return;
  }

  if (placement.has_value()) {
    const auto& rect = placement->rect;
    const std::int32_t cursor =
        static_cast<std::int32_t>(placement->byte_offset);
    const std::string surrounding_text;
    zwp_text_input_v3_enable(text_input_);
    zwp_text_input_v3_set_surrounding_text(
        text_input_,
        surrounding_text.c_str(),
        cursor,
        cursor);
    zwp_text_input_v3_set_content_type(text_input_, 0, 0);
    zwp_text_input_v3_set_cursor_rectangle(
        text_input_,
        static_cast<std::int32_t>(rect.origin.x),
        static_cast<std::int32_t>(rect.origin.y),
        static_cast<std::int32_t>(rect.size.width),
        static_cast<std::int32_t>(rect.size.height));
  } else {
    zwp_text_input_v3_disable(text_input_);
  }
  zwp_text_input_v3_commit(text_input_);
  if (display_ != nullptr) {
    (void)wl_display_flush(display_);
  }
}

} // namespace cgpui
