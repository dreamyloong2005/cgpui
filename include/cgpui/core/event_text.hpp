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

// Conversion state of one composition clause. Windows maps IMM32
// ATTR_INPUT / ATTR_INPUT_ERROR to input, ATTR_TARGET_CONVERTED to
// selected_converted, ATTR_CONVERTED / ATTR_FIXEDCONVERTED to converted and
// ATTR_TARGET_NOTCONVERTED to selected_unconverted. Wayland text-input v3 has
// no clauses: one input span covers the preedit.
enum class ImeClauseStyle : std::uint8_t {
  input,
  selected_converted,
  converted,
  selected_unconverted,
};

// One clause of the preedit; offsets are UTF-8 bytes into
// ImeComposition::text.
struct ImePreeditStyleSpan {
  std::uint32_t byte_offset = 0;
  std::uint32_t byte_length = 0;
  ImePreeditStyleKind kind = ImePreeditStyleKind::underline;
  ImeClauseStyle clause = ImeClauseStyle::input;
};

inline constexpr std::size_t kImePreeditStyleSpanCapacity = 16;

struct ImeComposition {
  ImeCompositionPhase phase = ImeCompositionPhase::update;
  std::string text;
  KeyboardModifiers modifiers;
  std::uint32_t serial = 0;
  // Caret range inside the preedit in UTF-8 bytes; begin == end is a caret,
  // both -1 hide it (text-input v3). Windows reports a caret (GCS_CURSORPOS).
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
