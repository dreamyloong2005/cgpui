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
};

struct ImeDeleteSurroundingText {
  std::uint32_t before_length = 0;
  std::uint32_t after_length = 0;
  KeyboardModifiers modifiers;
};

} // namespace cgpui
