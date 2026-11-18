#pragma once

#include "cgpui/core/event_text.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace cgpui {

// One clause of an IME composition; offsets are UTF-8 bytes into
// TextComposition::text.
struct TextCompositionClause {
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  ImeClauseStyle style = ImeClauseStyle::input;
};

// Composition (preedit) text held by a TextModel while the IME composes.
// The caret range is in UTF-8 bytes inside `text`, on code point
// boundaries; `caret_visible` is false when the IME hides the caret.
struct TextComposition {
  std::string text;
  std::size_t caret_begin = 0;
  std::size_t caret_end = 0;
  bool caret_visible = true;
  std::vector<TextCompositionClause> clauses;
};

// Normalizes the platform event: clamps the caret and clause offsets to the
// text and snaps them back to code point boundaries, hides the caret when a
// cursor offset is negative, drops empty clauses, and uses one input clause
// over the whole text when the event has none.
[[nodiscard]] TextComposition text_composition_from_ime(
    const ImeComposition& composition);

// Plain composition text: caret at the end, one input clause.
[[nodiscard]] TextComposition text_composition_from_text(std::string text);

} // namespace cgpui
