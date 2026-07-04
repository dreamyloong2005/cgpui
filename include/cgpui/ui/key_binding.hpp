#pragma once

#include "cgpui/core/event_keyboard.hpp"
#include "cgpui/ui/text_edit_actions.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

struct KeyBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  std::string action_name;
};

struct TextEditBinding {
  std::uint32_t key_code = 0;
  KeyAction action = KeyAction::pressed;
  KeyboardModifiers modifiers;
  TextEditAction edit_action = TextEditAction::move_previous;
};

[[nodiscard]] std::optional<KeyBinding> parse_key_binding(
    std::string_view grammar,
    std::string action_name);

} // namespace cgpui
