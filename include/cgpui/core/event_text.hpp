#pragma once

#include "cgpui/core/event_keyboard.hpp"

#include <array>
#include <cstdint>
#include <cstddef>
#include <string>

namespace cgpui {

struct TextInput {
  std::string text;
  KeyboardModifiers modifiers;
  bool composed = false;
};

enum class ImeCompositionPhase {
  update,
  commit,
  cancel,
};

enum class ImePreeditStyleKind : std::uint8_t {
  underline,
  highlight,
};

struct ImePreeditStyleSpan {
  std::uint32_t byte_offset = 0;
  std::uint32_t byte_length = 0;
  ImePreeditStyleKind kind = ImePreeditStyleKind::underline;
};

inline constexpr std::size_t kImePreeditStyleSpanCapacity = 4;

struct ImeComposition {
  ImeCompositionPhase phase = ImeCompositionPhase::update;
  std::string text;
  KeyboardModifiers modifiers;
  std::uint32_t serial = 0;
  std::int32_t preedit_cursor_begin = 0;
  std::int32_t preedit_cursor_end = 0;
  std::array<ImePreeditStyleSpan, kImePreeditStyleSpanCapacity>
      preedit_styles{};
  std::size_t preedit_style_count = 0;
};

[[nodiscard]] bool append_ime_preedit_style(
    ImeComposition& composition,
    ImePreeditStyleSpan style);
[[nodiscard]] bool append_ime_default_preedit_style(
    ImeComposition& composition);

struct ImeDeleteSurroundingText {
  std::uint32_t before_length = 0;
  std::uint32_t after_length = 0;
  KeyboardModifiers modifiers;
  std::uint32_t serial = 0;
};

} // namespace cgpui
