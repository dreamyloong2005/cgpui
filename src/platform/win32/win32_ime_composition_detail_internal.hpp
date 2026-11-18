#pragma once

#include "cgpui/core/event_text.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

// Raw IMM32 composition data in UTF-16 units, as returned by
// ImmGetCompositionStringW. Kept free of IMM calls so the conversion to the
// public UTF-8 event is testable with synthetic data.
struct Win32ImeCompositionDetail {
  std::wstring text;
  // GCS_CURSORPOS; nullopt when the IME reports none (caret at the end).
  std::optional<std::int32_t> cursor;
  // GCS_COMPATTR: one ATTR_* byte per UTF-16 unit.
  std::vector<std::uint8_t> attributes;
  // GCS_COMPCLAUSE: clause boundaries in UTF-16 units, 0 first, size last.
  std::vector<std::uint32_t> clauses;
};

[[nodiscard]] ImeClauseStyle win32_ime_clause_style(std::uint8_t attribute);

// UTF-8 byte offset of a UTF-16 offset; an offset inside a surrogate pair
// maps to the start of the pair, offsets past the end to the UTF-8 size.
[[nodiscard]] std::size_t win32_utf8_offset_for_utf16(
    std::wstring_view text,
    std::size_t utf16_offset);

// Fills phase update, text, caret and clause spans. Clauses beyond
// kImePreeditStyleSpanCapacity merge into the last span.
[[nodiscard]] ImeComposition win32_ime_composition_update(
    const Win32ImeCompositionDetail& detail,
    KeyboardModifiers modifiers);

} // namespace cgpui
