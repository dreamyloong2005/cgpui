#include "wayland_text_input_internal.hpp"

namespace cgpui {

void WaylandTextInput::handle_enter(
    void* data,
    zwp_text_input_v3* text_input,
    wl_surface* surface) {
  (void)text_input;
  auto* self = static_cast<WaylandTextInput*>(data);
  self->active_window_ =
      self->find_window_ ? self->find_window_(surface) : nullptr;
  if (self->active_window_ != nullptr) {
    wayland_window_text_input_entered(*self->active_window_);
  }
}

void WaylandTextInput::handle_leave(
    void* data,
    zwp_text_input_v3* text_input,
    wl_surface* surface) {
  (void)text_input;
  auto* self = static_cast<WaylandTextInput*>(data);
  WaylandWindow* window =
      self->find_window_ ? self->find_window_(surface) : self->active_window_;
  if (window != nullptr) {
    wayland_window_text_input_left(*window, self->current_modifiers());
  }
  if (window == self->active_window_) {
    self->active_window_ = nullptr;
  }
  self->pending_preedit_.reset();
  self->pending_commit_.reset();
  self->pending_delete_surrounding_.reset();
}

void WaylandTextInput::handle_preedit_string(
    void* data,
    zwp_text_input_v3* text_input,
    const char* text,
    std::int32_t cursor_begin,
    std::int32_t cursor_end) {
  (void)text_input;
  (void)cursor_begin;
  (void)cursor_end;
  auto* self = static_cast<WaylandTextInput*>(data);
  self->pending_preedit_ = text == nullptr ? std::string{} : std::string(text);
}

void WaylandTextInput::handle_commit_string(
    void* data,
    zwp_text_input_v3* text_input,
    const char* text) {
  (void)text_input;
  auto* self = static_cast<WaylandTextInput*>(data);
  self->pending_commit_ = text == nullptr ? std::string{} : std::string(text);
}

void WaylandTextInput::handle_delete_surrounding_text(
    void* data,
    zwp_text_input_v3* text_input,
    std::uint32_t before_length,
    std::uint32_t after_length) {
  (void)text_input;
  auto* self = static_cast<WaylandTextInput*>(data);
  self->pending_delete_surrounding_ =
      PendingDeleteSurroundingText{
          .before_length = before_length,
          .after_length = after_length,
      };
}

void WaylandTextInput::handle_done(
    void* data,
    zwp_text_input_v3* text_input,
    std::uint32_t serial) {
  (void)text_input;
  auto* self = static_cast<WaylandTextInput*>(data);
  if (self->active_window_ == nullptr) {
    self->pending_preedit_.reset();
    self->pending_commit_.reset();
    return;
  }

  const KeyboardModifiers modifiers = self->current_modifiers();
  if (self->pending_preedit_.has_value()) {
    wayland_window_text_input_preedit(
        *self->active_window_,
        std::move(*self->pending_preedit_),
        modifiers,
        serial);
  }
  if (self->pending_delete_surrounding_.has_value()) {
    wayland_window_text_input_delete_surrounding(
        *self->active_window_,
        self->pending_delete_surrounding_->before_length,
        self->pending_delete_surrounding_->after_length,
        modifiers,
        serial);
  }
  if (self->pending_commit_.has_value()) {
    wayland_window_text_input_commit(
        *self->active_window_,
        std::move(*self->pending_commit_),
        modifiers,
        serial);
  }
  self->pending_preedit_.reset();
  self->pending_commit_.reset();
  self->pending_delete_surrounding_.reset();
}

KeyboardModifiers WaylandTextInput::current_modifiers() const {
  return modifiers_ ? modifiers_() : KeyboardModifiers{};
}

} // namespace cgpui
