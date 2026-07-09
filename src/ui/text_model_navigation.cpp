#include "cgpui/ui/text_model.hpp"

#include <algorithm>

namespace cgpui {

std::size_t TextModel::line_count() const {
  return static_cast<std::size_t>(
      std::count(text_.begin(), text_.end(), '\n')) + 1U;
}

std::size_t TextModel::line_index_at(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  return static_cast<std::size_t>(
      std::count(text_.begin(), text_.begin() + clamped, '\n'));
}

std::size_t TextModel::line_start_offset(std::size_t offset) const {
  return line_start_for_offset(offset);
}

std::size_t TextModel::line_end_offset(std::size_t offset) const {
  return line_end_for_offset(offset);
}

TextSelectionRange TextModel::word_selection_range_at(
    std::size_t offset) const {
  std::size_t target = clamp_offset(offset);
  if (target >= text_.size()) {
    return TextSelectionRange{
        .start = target,
        .end = target,
        .collapsed = true,
    };
  }
  if (target > 0 && is_utf8_continuation(text_[target])) {
    target = previous_codepoint_boundary(target);
  }
  if (grapheme_at_is_word_separator(target)) {
    return TextSelectionRange{
        .start = target,
        .end = target,
        .collapsed = true,
    };
  }

  const std::size_t start = previous_word_boundary(target);
  const std::size_t end = next_word_boundary(target);
  return TextSelectionRange{
      .start = start,
      .end = end,
      .collapsed = start == end,
  };
}

TextSelectionRange TextModel::line_selection_range_at(
    std::size_t offset) const {
  const std::size_t target = clamp_offset(offset);
  const std::size_t start = line_start_for_offset(target);
  std::size_t end = line_end_for_offset(target);
  if (end > start && text_[end - 1] == '\r') {
    end -= 1;
  }
  return TextSelectionRange{
      .start = start,
      .end = end,
      .collapsed = start == end,
  };
}

bool TextModel::move_cursor_previous() {
  if (cursor_ == 0) {
    return false;
  }
  cursor_ = previous_grapheme_boundary(cursor_);
  collapse_selection_to_cursor();
  return true;
}

bool TextModel::move_cursor_next() {
  if (cursor_ >= text_.size()) {
    return false;
  }
  cursor_ = next_grapheme_boundary(cursor_);
  collapse_selection_to_cursor();
  return true;
}

bool TextModel::move_cursor_previous_word() {
  if (cursor_ == 0) {
    return false;
  }
  const std::size_t previous = previous_word_boundary(cursor_);
  if (previous == cursor_) {
    return false;
  }
  cursor_ = previous;
  collapse_selection_to_cursor();
  return true;
}

bool TextModel::move_cursor_next_word() {
  if (cursor_ >= text_.size()) {
    return false;
  }
  const std::size_t next = next_word_boundary(cursor_);
  if (next == cursor_) {
    return false;
  }
  cursor_ = next;
  collapse_selection_to_cursor();
  return true;
}

bool TextModel::move_cursor_line_start() {
  return move_cursor_to(line_start_for_offset(cursor_));
}

bool TextModel::move_cursor_line_end() {
  return move_cursor_to(line_end_for_offset(cursor_));
}

bool TextModel::move_cursor_previous_line() {
  const std::size_t target = previous_line_offset(cursor_);
  if (target == cursor_) {
    return false;
  }
  return move_cursor_to(target);
}

bool TextModel::move_cursor_next_line() {
  const std::size_t target = next_line_offset(cursor_);
  if (target == cursor_) {
    return false;
  }
  return move_cursor_to(target);
}

bool TextModel::is_utf8_continuation(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

std::size_t TextModel::line_start_for_offset(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  if (clamped == 0 || text_.empty()) {
    return 0;
  }
  const std::size_t search_from = clamped - 1;
  const std::size_t newline = text_.rfind('\n', search_from);
  return newline == std::string::npos ? 0 : newline + 1;
}

std::size_t TextModel::line_end_for_offset(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  const std::size_t newline = text_.find('\n', clamped);
  return newline == std::string::npos ? text_.size() : newline;
}

std::size_t TextModel::line_column_for_offset(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  return clamped - line_start_for_offset(clamped);
}

std::size_t TextModel::previous_line_offset(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  const std::size_t current_start = line_start_for_offset(clamped);
  if (current_start == 0) {
    return clamped;
  }
  const std::size_t column = line_column_for_offset(clamped);
  const std::size_t previous_line_position = current_start - 1;
  const std::size_t previous_start =
      line_start_for_offset(previous_line_position);
  const std::size_t previous_end =
      line_end_for_offset(previous_line_position);
  return std::min(previous_start + column, previous_end);
}

std::size_t TextModel::next_line_offset(std::size_t offset) const {
  const std::size_t clamped = clamp_offset(offset);
  const std::size_t current_end = line_end_for_offset(clamped);
  if (current_end >= text_.size()) {
    return clamped;
  }
  const std::size_t column = line_column_for_offset(clamped);
  const std::size_t next_start = current_end + 1;
  const std::size_t next_end = line_end_for_offset(next_start);
  return std::min(next_start + column, next_end);
}

std::size_t TextModel::previous_codepoint_boundary(std::size_t offset) const {
  std::size_t index = offset - 1;
  while (index > 0 && is_utf8_continuation(text_[index])) {
    index -= 1;
  }
  return index;
}

std::size_t TextModel::next_codepoint_boundary(std::size_t offset) const {
  std::size_t index = offset + 1;
  while (index < text_.size() && is_utf8_continuation(text_[index])) {
    index += 1;
  }
  return index;
}

TextModel::DecodedCodepoint TextModel::codepoint_at(std::size_t offset) const {
  if (offset >= text_.size()) {
    return {};
  }
  const auto first = static_cast<unsigned char>(text_[offset]);
  if (first < 0x80U) {
    return DecodedCodepoint{.value = first, .next = offset + 1};
  }

  const std::size_t next = next_codepoint_boundary(offset);
  if (next - offset == 2) {
    const auto second = static_cast<unsigned char>(text_[offset + 1]);
    return DecodedCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x1FU) << 6U) |
            (static_cast<std::uint32_t>(second) & 0x3FU),
        .next = next,
    };
  }
  if (next - offset == 3) {
    const auto second = static_cast<unsigned char>(text_[offset + 1]);
    const auto third = static_cast<unsigned char>(text_[offset + 2]);
    return DecodedCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x0FU) << 12U) |
            ((static_cast<std::uint32_t>(second) & 0x3FU) << 6U) |
            (static_cast<std::uint32_t>(third) & 0x3FU),
        .next = next,
    };
  }
  if (next - offset == 4) {
    const auto second = static_cast<unsigned char>(text_[offset + 1]);
    const auto third = static_cast<unsigned char>(text_[offset + 2]);
    const auto fourth = static_cast<unsigned char>(text_[offset + 3]);
    return DecodedCodepoint{
        .value = ((static_cast<std::uint32_t>(first) & 0x07U) << 18U) |
            ((static_cast<std::uint32_t>(second) & 0x3FU) << 12U) |
            ((static_cast<std::uint32_t>(third) & 0x3FU) << 6U) |
            (static_cast<std::uint32_t>(fourth) & 0x3FU),
        .next = next,
    };
  }
  return DecodedCodepoint{.value = first, .next = next};
}

bool TextModel::is_combining_mark(std::uint32_t value) {
  return (value >= 0x0300U && value <= 0x036FU) ||
      (value >= 0x1AB0U && value <= 0x1AFFU) ||
      (value >= 0x1DC0U && value <= 0x1DFFU) ||
      (value >= 0x20D0U && value <= 0x20FFU) ||
      (value >= 0xFE20U && value <= 0xFE2FU);
}

bool TextModel::is_variation_selector(std::uint32_t value) {
  return (value >= 0xFE00U && value <= 0xFE0FU) ||
      (value >= 0xE0100U && value <= 0xE01EFU);
}

bool TextModel::is_regional_indicator(std::uint32_t value) {
  return value >= 0x1F1E6U && value <= 0x1F1FFU;
}

bool TextModel::is_zero_width_joiner(std::uint32_t value) {
  return value == 0x200DU;
}

bool TextModel::is_unicode_space(std::uint32_t value) {
  return (value >= 0x0009U && value <= 0x000DU) || value == 0x0020U ||
      value == 0x0085U || value == 0x00A0U || value == 0x1680U ||
      (value >= 0x2000U && value <= 0x200AU) || value == 0x2028U ||
      value == 0x2029U || value == 0x202FU || value == 0x205FU ||
      value == 0x3000U;
}

bool TextModel::grapheme_at_is_word_separator(std::size_t offset) const {
  return offset >= text_.size() || is_unicode_space(codepoint_at(offset).value);
}

std::size_t TextModel::next_grapheme_boundary(std::size_t offset) const {
  const DecodedCodepoint first = codepoint_at(offset);
  if (first.next <= offset || first.next > text_.size()) {
    return std::min(offset + 1, text_.size());
  }

  std::size_t next = first.next;
  std::size_t regional_indicator_count =
      is_regional_indicator(first.value) ? 1U : 0U;
  while (next < text_.size()) {
    const DecodedCodepoint current = codepoint_at(next);
    if (current.next <= next || current.next > text_.size()) {
      break;
    }
    if (is_combining_mark(current.value) ||
        is_variation_selector(current.value)) {
      next = current.next;
      continue;
    }
    if (is_zero_width_joiner(current.value)) {
      next = current.next;
      if (next < text_.size()) {
        next = codepoint_at(next).next;
      }
      continue;
    }
    if (regional_indicator_count == 1U &&
        is_regional_indicator(current.value)) {
      next = current.next;
      regional_indicator_count += 1U;
      continue;
    }
    break;
  }
  return next;
}

std::size_t TextModel::previous_grapheme_boundary(std::size_t offset) const {
  std::size_t previous = 0;
  std::size_t next = 0;
  while (next < offset && next < text_.size()) {
    previous = next;
    next = next_grapheme_boundary(next);
    if (next <= previous) {
      break;
    }
  }
  return previous;
}

std::size_t TextModel::previous_word_boundary(std::size_t offset) const {
  std::size_t boundary = clamp_offset(offset);
  while (boundary > 0) {
    const std::size_t previous = previous_grapheme_boundary(boundary);
    if (!grapheme_at_is_word_separator(previous)) {
      break;
    }
    boundary = previous;
  }
  while (boundary > 0) {
    const std::size_t previous = previous_grapheme_boundary(boundary);
    if (grapheme_at_is_word_separator(previous)) {
      break;
    }
    boundary = previous;
  }
  return boundary;
}

std::size_t TextModel::next_word_boundary(std::size_t offset) const {
  std::size_t boundary = clamp_offset(offset);
  while (boundary < text_.size() && grapheme_at_is_word_separator(boundary)) {
    boundary = next_grapheme_boundary(boundary);
  }
  while (boundary < text_.size() && !grapheme_at_is_word_separator(boundary)) {
    boundary = next_grapheme_boundary(boundary);
  }
  return boundary;
}

} // namespace cgpui
