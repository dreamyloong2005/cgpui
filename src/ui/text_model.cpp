#include "cgpui/ui/text_model.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

TextModel::TextModel(std::string text) : text_(std::move(text)) {
  cursor_ = text_.size();
  collapse_selection_to_cursor();
}

std::string_view TextModel::text() const {
  return text_;
}

std::size_t TextModel::cursor() const {
  return cursor_;
}

std::size_t TextModel::selection_anchor() const {
  return selection_anchor_;
}

std::size_t TextModel::selection_head() const {
  return selection_head_;
}

bool TextModel::has_composition() const {
  return has_composition_;
}

std::string_view TextModel::composition_text() const {
  return composition_text_;
}

void TextModel::set_composition_text(std::string_view text) {
  composition_text_ = std::string(text);
  has_composition_ = true;
}

void TextModel::commit_composition() {
  if (!has_composition_) {
    return;
  }
  std::string committed = std::move(composition_text_);
  clear_composition();
  insert_text(committed);
}

void TextModel::cancel_composition() {
  clear_composition();
}

void TextModel::insert_text(std::string_view text) {
  const TextHistorySnapshot before = history_snapshot();
  (void)erase_selection_if_needed();
  text_.insert(cursor_, text);
  cursor_ += text.size();
  collapse_selection_to_cursor();
  commit_history_record(before);
}

bool TextModel::backspace() {
  const TextHistorySnapshot before = history_snapshot();
  if (erase_selection_if_needed()) {
    commit_history_record(before);
    return true;
  }
  if (cursor_ == 0) {
    return false;
  }
  const std::size_t previous = previous_grapheme_boundary(cursor_);
  text_.erase(previous, cursor_ - previous);
  cursor_ = previous;
  collapse_selection_to_cursor();
  commit_history_record(before);
  return true;
}

bool TextModel::delete_forward() {
  const TextHistorySnapshot before = history_snapshot();
  if (erase_selection_if_needed()) {
    commit_history_record(before);
    return true;
  }
  if (cursor_ >= text_.size()) {
    return false;
  }
  const std::size_t next = next_grapheme_boundary(cursor_);
  text_.erase(cursor_, next - cursor_);
  collapse_selection_to_cursor();
  commit_history_record(before);
  return true;
}

bool TextModel::delete_surrounding_text(
    std::size_t before_length,
    std::size_t after_length) {
  const TextHistorySnapshot before = history_snapshot();
  const std::size_t cursor = clamp_offset(cursor_);
  std::size_t start = before_length > cursor ? 0 : cursor - before_length;
  while (start > 0 && is_utf8_continuation(text_[start])) {
    start -= 1;
  }

  const std::size_t available_after = text_.size() - cursor;
  std::size_t end = cursor + std::min(after_length, available_after);
  while (end < text_.size() && is_utf8_continuation(text_[end])) {
    end += 1;
  }

  if (start == end) {
    return false;
  }
  text_.erase(start, end - start);
  cursor_ = start;
  collapse_selection_to_cursor();
  clear_composition();
  commit_history_record(before);
  return true;
}

bool TextModel::apply_edit_action(TextEditAction action) {
  switch (action) {
    case TextEditAction::move_previous:
      return move_cursor_previous();
    case TextEditAction::move_next:
      return move_cursor_next();
    case TextEditAction::extend_previous:
      return extend_selection_previous();
    case TextEditAction::extend_next:
      return extend_selection_next();
    case TextEditAction::move_previous_word:
      return move_cursor_previous_word();
    case TextEditAction::move_next_word:
      return move_cursor_next_word();
    case TextEditAction::extend_previous_word:
      return extend_selection_previous_word();
    case TextEditAction::extend_next_word:
      return extend_selection_next_word();
    case TextEditAction::move_line_start:
      return move_cursor_line_start();
    case TextEditAction::move_line_end:
      return move_cursor_line_end();
    case TextEditAction::move_previous_line:
      return move_cursor_previous_line();
    case TextEditAction::move_next_line:
      return move_cursor_next_line();
    case TextEditAction::extend_line_start:
      return extend_selection_line_start();
    case TextEditAction::extend_line_end:
      return extend_selection_line_end();
    case TextEditAction::extend_previous_line:
      return extend_selection_previous_line();
    case TextEditAction::extend_next_line:
      return extend_selection_next_line();
    case TextEditAction::undo:
      return undo();
    case TextEditAction::redo:
      return redo();
    case TextEditAction::backspace:
      return backspace();
    case TextEditAction::delete_forward:
      return delete_forward();
  }
  return false;
}

void TextModel::clear_composition() {
  composition_text_.clear();
  has_composition_ = false;
}

std::size_t TextModel::clamp_offset(std::size_t offset) const {
  return std::min(offset, text_.size());
}

void TextModel::collapse_selection_to_cursor() {
  selection_anchor_ = cursor_;
  selection_head_ = cursor_;
}

bool TextModel::move_cursor_to(std::size_t offset) {
  const std::size_t target = clamp_offset(offset);
  if (target == cursor_) {
    return false;
  }
  cursor_ = target;
  collapse_selection_to_cursor();
  return true;
}

} // namespace cgpui
