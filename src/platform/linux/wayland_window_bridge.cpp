#include "wayland_window_internal.hpp"

namespace cgpui {

wl_surface* wayland_window_surface(WaylandWindow& window) {
  return window.surface();
}

WindowState wayland_window_state(const WaylandWindow& window) {
  return window.state();
}

CursorShape wayland_window_cursor_shape(const WaylandWindow& window) {
  return window.cursor_shape();
}

void wayland_window_set_cursor(WaylandWindow& window, CursorShape cursor_shape) {
  window.set_cursor(cursor_shape);
}

void wayland_window_set_ime_text_input_placement(
    WaylandWindow& window,
    std::optional<ImeTextInputPlacement> placement) {
  window.set_ime_text_input_placement(std::move(placement));
}

void wayland_window_pointer_moved(WaylandWindow& window, Point position) {
  window.pointer_moved(position);
}

void wayland_window_pointer_button(
    WaylandWindow& window,
    MouseButton button,
    bool pressed,
    Point position) {
  window.pointer_button(button, pressed, position);
}

void wayland_window_pointer_scrolled(
    WaylandWindow& window,
    Point delta,
    Point position) {
  window.pointer_scrolled(delta, position);
}

void wayland_window_drag_entered(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  window.drag_entered(position, std::move(payload), action);
}

void wayland_window_drag_updated(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  window.drag_updated(position, std::move(payload), action);
}

void wayland_window_drag_dropped(
    WaylandWindow& window,
    Point position,
    DragDropPayload payload,
    DragDropAction action) {
  window.drag_dropped(position, std::move(payload), action);
}

void wayland_window_drag_exited(WaylandWindow& window, Point position) {
  window.drag_exited(position);
}

void wayland_window_wakeup_requested(WaylandWindow& window) {
  window.wakeup_requested();
}

void wayland_window_keyboard_key(
    WaylandWindow& window,
    std::uint32_t key,
    KeyAction action,
    KeyboardModifiers modifiers) {
  window.keyboard_key(key, action, modifiers);
}

void wayland_window_text_input(
    WaylandWindow& window,
    std::string text,
    KeyboardModifiers modifiers) {
  window.text_input(std::move(text), modifiers);
}

void wayland_window_text_input_entered(WaylandWindow& window) {
  window.text_input_entered();
}

void wayland_window_text_input_left(
    WaylandWindow& window,
    KeyboardModifiers modifiers) {
  window.text_input_left(modifiers);
}

void wayland_window_text_input_preedit(
    WaylandWindow& window,
    std::string text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  window.text_input_preedit(
      std::move(text),
      cursor_begin,
      cursor_end,
      modifiers,
      serial);
}

void wayland_window_text_input_commit(
    WaylandWindow& window,
    std::string text,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  window.text_input_commit(std::move(text), modifiers, serial);
}

void wayland_window_text_input_delete_surrounding(
    WaylandWindow& window,
    std::uint32_t before_length,
    std::uint32_t after_length,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  window.text_input_delete_surrounding(
      before_length,
      after_length,
      modifiers,
      serial);
}

void wayland_window_text_input_surrounding_text(
    WaylandWindow& window,
    std::string text,
    std::int32_t cursor,
    std::int32_t anchor) {
  window.text_input_surrounding_text(std::move(text), cursor, anchor);
}

void wayland_window_text_input_content_type(
    WaylandWindow& window,
    std::uint32_t hint,
    std::uint32_t purpose) {
  window.text_input_content_type(hint, purpose);
}

void wayland_window_focus_changed(WaylandWindow& window, bool focused) {
  window.focus_changed(focused);
}

void wayland_window_output_scale_changed(
    WaylandWindow& window,
    wl_output* output,
    std::int32_t scale,
    bool present) {
  window.output_scale_changed(output, scale, present);
}

} // namespace cgpui
