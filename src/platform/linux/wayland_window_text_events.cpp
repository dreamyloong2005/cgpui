#include "wayland_window_internal.hpp"

namespace cgpui {

void WaylandWindow::set_ime_text_input_placement(
    std::optional<ImeTextInputPlacement> placement) {
  text_input_state_.set_placement(placement);
  sync_text_input_state();
}

void WaylandWindow::text_input(
    std::string text,
    KeyboardModifiers modifiers) {
  callback_(TextInput{
      .text = std::move(text),
      .modifiers = modifiers});
}

void WaylandWindow::text_input_entered() {
  text_input_state_.enter();
}

void WaylandWindow::text_input_left(KeyboardModifiers modifiers) {
  if (auto event = text_input_state_.leave(modifiers); event.has_value()) {
    callback_(*event);
  }
}

void WaylandWindow::text_input_preedit(
    std::string text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  ImeComposition event =
      text_input_state_.preedit(std::move(text), modifiers);
  event.serial = serial;
  event.preedit_cursor_begin = cursor_begin;
  event.preedit_cursor_end = cursor_end;
  callback_(std::move(event));
}

void WaylandWindow::text_input_commit(
    std::string text,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  ImeComposition event =
      text_input_state_.commit(std::move(text), modifiers);
  event.serial = serial;
  callback_(std::move(event));
}

void WaylandWindow::text_input_delete_surrounding(
    std::uint32_t before_length,
    std::uint32_t after_length,
    KeyboardModifiers modifiers,
    std::uint32_t serial) {
  callback_(ImeDeleteSurroundingText{
      .before_length = before_length,
      .after_length = after_length,
      .modifiers = modifiers,
      .serial = serial});
}

void WaylandWindow::text_input_surrounding_text(
    std::string text,
    std::int32_t cursor,
    std::int32_t anchor) {
  text_input_state_.set_surrounding_text(std::move(text), cursor, anchor);
}

void WaylandWindow::text_input_content_type(
    std::uint32_t hint,
    std::uint32_t purpose) {
  text_input_state_.set_content_type(hint, purpose);
}

void WaylandWindow::set_text_input_available(bool available) {
  text_input_state_.set_available(available);
  sync_text_input_state();
}

void WaylandWindow::sync_text_input_state() {
  state_.ime_text_input_support = text_input_state_.support();
  state_.ime_text_input_placement = text_input_state_.placement();
}

} // namespace cgpui
