#include "cgpui/ui/text_model.hpp"

#include <algorithm>

namespace cgpui {

TextSelectionRange TextModel::selection() const {
  const std::size_t start = std::min(selection_anchor_, selection_head_);
  const std::size_t end = std::max(selection_anchor_, selection_head_);
  return TextSelectionRange{
      .start = start,
      .end = end,
      .collapsed = start == end,
  };
}

std::string TextModel::selected_text() const {
  const TextSelectionRange range = selection();
  if (range.collapsed) {
    return {};
  }
  return text_.substr(range.start, range.end - range.start);
}

void TextModel::set_selection(std::size_t anchor, std::size_t head) {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  selection_anchor_ = clamp_offset(anchor);
  selection_head_ = clamp_offset(head);
  cursor_ = selection_head_;
}

void TextModel::clear_selection() {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  collapse_selection_to_cursor();
}

bool TextModel::extend_selection_to(std::size_t offset) {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  const std::size_t target = clamp_offset(offset);
  if (target == cursor_) {
    return false;
  }
  cursor_ = target;
  selection_head_ = cursor_;
  return true;
}

bool TextModel::erase_selection_if_needed() {
  const TextSelectionRange range = selection();
  if (range.collapsed) {
    return false;
  }
  text_.erase(range.start, range.end - range.start);
  cursor_ = range.start;
  collapse_selection_to_cursor();
  return true;
}

bool TextModel::extend_selection_previous() {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  if (cursor_ == 0) {
    return false;
  }
  cursor_ = previous_grapheme_boundary(cursor_);
  selection_head_ = cursor_;
  return true;
}

bool TextModel::extend_selection_next() {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  if (cursor_ >= text_.size()) {
    return false;
  }
  cursor_ = next_grapheme_boundary(cursor_);
  selection_head_ = cursor_;
  return true;
}

bool TextModel::extend_selection_previous_word() {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  if (cursor_ == 0) {
    return false;
  }
  const std::size_t previous = previous_word_boundary(cursor_);
  if (previous == cursor_) {
    return false;
  }
  cursor_ = previous;
  selection_head_ = cursor_;
  return true;
}

bool TextModel::extend_selection_next_word() {
  clear_edit_history_grouping();
  clear_preferred_line_column();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  if (cursor_ >= text_.size()) {
    return false;
  }
  const std::size_t next = next_word_boundary(cursor_);
  if (next == cursor_) {
    return false;
  }
  cursor_ = next;
  selection_head_ = cursor_;
  return true;
}

bool TextModel::extend_selection_line_start() {
  return extend_selection_to(line_start_for_offset(cursor_));
}

bool TextModel::extend_selection_line_end() {
  return extend_selection_to(line_end_for_offset(cursor_));
}

bool TextModel::extend_selection_previous_line() {
  clear_edit_history_grouping();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  (void)preferred_line_column_for_vertical_navigation();
  const std::size_t target = previous_line_offset(cursor_);
  if (target == cursor_) {
    return false;
  }
  cursor_ = target;
  selection_head_ = cursor_;
  return true;
}

bool TextModel::extend_selection_next_line() {
  clear_edit_history_grouping();
  if (selection().collapsed) {
    selection_anchor_ = cursor_;
  }
  (void)preferred_line_column_for_vertical_navigation();
  const std::size_t target = next_line_offset(cursor_);
  if (target == cursor_) {
    return false;
  }
  cursor_ = target;
  selection_head_ = cursor_;
  return true;
}

} // namespace cgpui
