#include "win32_ime_composition_detail_internal.hpp"

#include "win32_internal.hpp"

#include <imm.h>

#include <algorithm>

namespace cgpui {
namespace {

bool is_high_surrogate(wchar_t value) {
  return value >= 0xD800 && value <= 0xDBFF;
}

bool is_low_surrogate(wchar_t value) {
  return value >= 0xDC00 && value <= 0xDFFF;
}

struct UnitRange {
  std::size_t begin = 0;
  std::size_t end = 0;
};

// Clause ranges in UTF-16 units: GCS_COMPCLAUSE boundaries when present,
// else runs of equal GCS_COMPATTR bytes, else the whole string.
std::vector<UnitRange> clause_ranges(const Win32ImeCompositionDetail& detail) {
  const std::size_t size = detail.text.size();
  std::vector<UnitRange> ranges;
  if (detail.clauses.size() >= 2) {
    for (std::size_t index = 0; index + 1 < detail.clauses.size(); ++index) {
      const std::size_t begin = (std::min)(
          static_cast<std::size_t>(detail.clauses[index]), size);
      const std::size_t end = (std::min)(
          static_cast<std::size_t>(detail.clauses[index + 1]), size);
      if (begin < end) {
        ranges.push_back(UnitRange{.begin = begin, .end = end});
      }
    }
    return ranges;
  }
  const std::size_t attributed = (std::min)(detail.attributes.size(), size);
  std::size_t begin = 0;
  for (std::size_t index = 1; index <= attributed; ++index) {
    if (index == attributed ||
        detail.attributes[index] != detail.attributes[begin]) {
      ranges.push_back(UnitRange{.begin = begin, .end = index});
      begin = index;
    }
  }
  if (begin < size) {
    ranges.push_back(UnitRange{.begin = begin, .end = size});
  }
  return ranges;
}

} // namespace

ImeClauseStyle win32_ime_clause_style(std::uint8_t attribute) {
  switch (attribute) {
    case ATTR_TARGET_CONVERTED:
      return ImeClauseStyle::selected_converted;
    case ATTR_CONVERTED:
    case ATTR_FIXEDCONVERTED:
      return ImeClauseStyle::converted;
    case ATTR_TARGET_NOTCONVERTED:
      return ImeClauseStyle::selected_unconverted;
    default:
      // ATTR_INPUT, ATTR_INPUT_ERROR and unknown values.
      return ImeClauseStyle::input;
  }
}

std::size_t win32_utf8_offset_for_utf16(
    std::wstring_view text,
    std::size_t utf16_offset) {
  const std::size_t end = (std::min)(utf16_offset, text.size());
  std::size_t bytes = 0;
  for (std::size_t index = 0; index < end; ++index) {
    const wchar_t unit = text[index];
    if (is_high_surrogate(unit) && index + 1 < text.size() &&
        is_low_surrogate(text[index + 1])) {
      if (index + 1 == end) {
        return bytes;
      }
      bytes += 4;
      index += 1;
    } else if (unit < 0x80) {
      bytes += 1;
    } else if (unit < 0x800) {
      bytes += 2;
    } else {
      // BMP characters and lone surrogates (encoded as U+FFFD).
      bytes += 3;
    }
  }
  return bytes;
}

ImeComposition win32_ime_composition_update(
    const Win32ImeCompositionDetail& detail,
    KeyboardModifiers modifiers) {
  ImeComposition composition{
      .phase = ImeCompositionPhase::update,
      .text = utf8_from_utf16(detail.text),
      .modifiers = modifiers,
  };
  const std::size_t cursor_units = detail.cursor.has_value()
      ? static_cast<std::size_t>((std::max)(*detail.cursor, 0))
      : detail.text.size();
  const auto caret = static_cast<std::int32_t>(
      win32_utf8_offset_for_utf16(detail.text, cursor_units));
  composition.preedit_cursor_begin = caret;
  composition.preedit_cursor_end = caret;

  for (const UnitRange& range : clause_ranges(detail)) {
    const auto begin = static_cast<std::uint32_t>(
        win32_utf8_offset_for_utf16(detail.text, range.begin));
    const auto end = static_cast<std::uint32_t>(
        win32_utf8_offset_for_utf16(detail.text, range.end));
    const ImeClauseStyle style = range.begin < detail.attributes.size()
        ? win32_ime_clause_style(detail.attributes[range.begin])
        : ImeClauseStyle::input;
    const ImePreeditStyleSpan span{
        .byte_offset = begin,
        .byte_length = end - begin,
        .kind = ImePreeditStyleKind::underline,
        .clause = style,
    };
    if (!append_ime_preedit_style(composition, span)) {
      ImePreeditStyleSpan& last =
          composition.preedit_styles[composition.preedit_style_count - 1];
      last.byte_length = end - last.byte_offset;
    }
  }
  return composition;
}

} // namespace cgpui
