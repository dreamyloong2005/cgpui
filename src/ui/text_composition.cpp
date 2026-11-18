#include "cgpui/ui/text_composition.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {
namespace {

bool is_utf8_continuation(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

std::size_t code_point_floor(const std::string& text, std::size_t offset) {
  offset = (std::min)(offset, text.size());
  while (offset > 0 && offset < text.size() &&
         is_utf8_continuation(text[offset])) {
    offset -= 1;
  }
  return offset;
}

void add_whole_text_clause(TextComposition& composition) {
  if (composition.clauses.empty() && !composition.text.empty()) {
    composition.clauses.push_back(TextCompositionClause{
        .byte_offset = 0,
        .byte_length = composition.text.size(),
    });
  }
}

} // namespace

TextComposition text_composition_from_ime(const ImeComposition& composition) {
  TextComposition result{.text = composition.text};
  const std::string& text = result.text;
  result.caret_visible = composition.preedit_cursor_begin >= 0 &&
                         composition.preedit_cursor_end >= 0;
  if (result.caret_visible) {
    const auto begin = code_point_floor(
        text, static_cast<std::size_t>(composition.preedit_cursor_begin));
    const auto end = code_point_floor(
        text, static_cast<std::size_t>(composition.preedit_cursor_end));
    result.caret_begin = (std::min)(begin, end);
    result.caret_end = (std::max)(begin, end);
  }
  const std::size_t count = (std::min)(composition.preedit_style_count,
                                       composition.preedit_styles.size());
  for (std::size_t index = 0; index < count; ++index) {
    const ImePreeditStyleSpan& span = composition.preedit_styles[index];
    const std::size_t begin = code_point_floor(text, span.byte_offset);
    const std::size_t end = code_point_floor(
        text, static_cast<std::size_t>(span.byte_offset) + span.byte_length);
    if (begin < end) {
      result.clauses.push_back(TextCompositionClause{
          .byte_offset = begin,
          .byte_length = end - begin,
          .style = span.clause,
      });
    }
  }
  add_whole_text_clause(result);
  return result;
}

TextComposition text_composition_from_text(std::string text) {
  TextComposition result{.text = std::move(text)};
  result.caret_begin = result.text.size();
  result.caret_end = result.text.size();
  add_whole_text_clause(result);
  return result;
}

} // namespace cgpui
