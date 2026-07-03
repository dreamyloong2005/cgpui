#pragma once

#include "cgpui/ui/runtime.hpp"

#include <cstdint>
#include <optional>
#include <variant>

namespace cgpui {
namespace {

inline bool is_keyboard_routed_event(const PlatformEvent& event) {
  return std::holds_alternative<KeyboardKey>(event) ||
         std::holds_alternative<TextInput>(event) ||
         std::holds_alternative<ImeComposition>(event) ||
         std::holds_alternative<ImeDeleteSurroundingText>(event);
}

inline bool is_focus_activation_event(const PlatformEvent& event) {
  const auto* button = std::get_if<PointerButton>(&event);
  return button != nullptr && button->button == MouseButton::left &&
         button->pressed;
}

inline bool is_focus_traversal_key(const KeyboardKey& key) {
  constexpr std::uint32_t tab_key_code = 9;
  return key.key_code == tab_key_code && key.action == KeyAction::pressed &&
         !key.modifiers.control && !key.modifiers.alt && !key.modifiers.super;
}

inline bool modifiers_equal(KeyboardModifiers lhs, KeyboardModifiers rhs) {
  return lhs.shift == rhs.shift && lhs.control == rhs.control &&
         lhs.alt == rhs.alt && lhs.super == rhs.super;
}

inline bool rect_equal(Rect lhs, Rect rhs) {
  return lhs.origin.x == rhs.origin.x && lhs.origin.y == rhs.origin.y &&
         lhs.size.width == rhs.size.width && lhs.size.height == rhs.size.height;
}

inline bool ime_text_input_placement_equal(
    const std::optional<ImeTextInputPlacement>& lhs,
    const std::optional<ImeTextInputPlacement>& rhs) {
  if (lhs.has_value() != rhs.has_value()) {
    return false;
  }
  if (!lhs.has_value()) {
    return true;
  }
  return lhs->byte_offset == rhs->byte_offset &&
         rect_equal(lhs->rect, rhs->rect);
}

} // namespace

} // namespace cgpui
