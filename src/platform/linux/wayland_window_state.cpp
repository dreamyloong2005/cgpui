#include "wayland_window_state.hpp"

#include "wayland_internal.hpp"

#include <utility>

namespace cgpui {

void WaylandTextInputState::set_available(bool available) {
  available_ = available;
}

ImeTextInputSupport WaylandTextInputState::support() const {
  return available_ ? ImeTextInputSupport::available
                    : ImeTextInputSupport::unsupported;
}

void WaylandTextInputState::set_placement(
    std::optional<ImeTextInputPlacement> placement) {
  placement_ = placement;
}

std::optional<ImeTextInputPlacement> WaylandTextInputState::placement() const {
  return placement_;
}

void WaylandTextInputState::enter() {
  entered_ = true;
}

std::optional<ImeComposition> WaylandTextInputState::leave(
    KeyboardModifiers modifiers) {
  entered_ = false;
  if (!preedit_text_.empty()) {
    preedit_text_.clear();
    return ImeComposition{
        .phase = ImeCompositionPhase::cancel,
        .text = {},
        .modifiers = modifiers};
  }
  return std::nullopt;
}

void WaylandTextInputState::set_surrounding_text(
    std::string surrounding_text,
    std::int32_t cursor,
    std::int32_t anchor) {
  surrounding_text_ = std::move(surrounding_text);
  surrounding_text_cursor_ = cursor;
  surrounding_text_anchor_ = anchor;
}

void WaylandTextInputState::set_content_type(
    std::uint32_t hint,
    std::uint32_t purpose) {
  content_type_hint_ = hint;
  content_type_purpose_ = purpose;
}

ImeComposition WaylandTextInputState::preedit(
    std::string text,
    KeyboardModifiers modifiers) {
  preedit_text_ = std::move(text);
  return ImeComposition{
      .phase = ImeCompositionPhase::update,
      .text = preedit_text_,
      .modifiers = modifiers};
}

ImeComposition WaylandTextInputState::commit(
    std::string text,
    KeyboardModifiers modifiers) {
  preedit_text_.clear();
  committed_text_ = std::move(text);
  return ImeComposition{
      .phase = ImeCompositionPhase::commit,
      .text = committed_text_,
      .modifiers = modifiers};
}

std::string cursor_name_for_shape(CursorShape shape) {
  switch (shape) {
  case CursorShape::default_arrow:
    return "left_ptr";
  case CursorShape::pointing_hand:
    return "hand2";
  case CursorShape::text:
    return "xterm";
  case CursorShape::crosshair:
    return "crosshair";
  case CursorShape::resize_left_right:
    return "sb_h_double_arrow";
  case CursorShape::resize_up_down:
    return "sb_v_double_arrow";
  case CursorShape::not_allowed:
    return "not-allowed";
  }
  return "left_ptr";
}

WaylandXdgToplevelState parse_xdg_toplevel_states(const wl_array* states) {
  WaylandXdgToplevelState result;
  if (states == nullptr || states->data == nullptr) {
    return result;
  }

  const auto* begin = static_cast<const std::uint32_t*>(states->data);
  const auto* end = reinterpret_cast<const std::uint32_t*>(
      static_cast<const char*>(states->data) + states->size);
  for (const auto* state = begin; state < end; ++state) {
    switch (*state) {
    case xdg_toplevel_state_activated:
      result.activated = true;
      break;
    case xdg_toplevel_state_maximized:
      result.maximized = true;
      break;
    case xdg_toplevel_state_fullscreen:
      result.fullscreen = true;
      break;
    default:
      break;
    }
  }
  return result;
}

} // namespace cgpui
