#pragma once

#include "cgpui/core/event_keyboard.hpp"

#include <cstdint>
#include <string>

namespace cgpui {

struct TextInput {
  std::string text;
  KeyboardModifiers modifiers;
};

enum class ImeCompositionPhase {
  update,
  commit,
  cancel,
};

struct ImeComposition {
  ImeCompositionPhase phase = ImeCompositionPhase::update;
  std::string text;
  KeyboardModifiers modifiers;
  std::uint32_t serial = 0;
  std::int32_t preedit_cursor_begin = 0;
  std::int32_t preedit_cursor_end = 0;
};

struct ImeDeleteSurroundingText {
  std::uint32_t before_length = 0;
  std::uint32_t after_length = 0;
  KeyboardModifiers modifiers;
  std::uint32_t serial = 0;
};

} // namespace cgpui
