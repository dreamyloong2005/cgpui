#include "cgpui/ui/text_model.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

bool TextModel::can_undo() const {
  return !undo_stack_.empty();
}

bool TextModel::can_redo() const {
  return !redo_stack_.empty();
}

bool TextModel::undo() {
  if (undo_stack_.empty()) {
    return false;
  }
  const TextEditHistoryRecord record = undo_stack_.back();
  undo_stack_.pop_back();
  redo_stack_.push_back(record);
  restore_history_snapshot(record.before);
  return true;
}

bool TextModel::redo() {
  if (redo_stack_.empty()) {
    return false;
  }
  const TextEditHistoryRecord record = redo_stack_.back();
  redo_stack_.pop_back();
  push_undo_record(record);
  restore_history_snapshot(record.after);
  return true;
}

TextModel::TextHistorySnapshot TextModel::history_snapshot() const {
  return TextHistorySnapshot{
      .text = text_,
      .cursor = cursor_,
      .selection_anchor = selection_anchor_,
      .selection_head = selection_head_,
  };
}

bool TextModel::history_snapshots_equal(
    const TextHistorySnapshot& left,
    const TextHistorySnapshot& right) {
  return left.text == right.text && left.cursor == right.cursor &&
      left.selection_anchor == right.selection_anchor &&
      left.selection_head == right.selection_head;
}

void TextModel::restore_history_snapshot(const TextHistorySnapshot& snapshot) {
  text_ = snapshot.text;
  cursor_ = std::min(snapshot.cursor, text_.size());
  selection_anchor_ = std::min(snapshot.selection_anchor, text_.size());
  selection_head_ = std::min(snapshot.selection_head, text_.size());
  clear_preferred_line_column();
  clear_composition();
}

void TextModel::push_undo_record(const TextEditHistoryRecord& record) {
  undo_stack_.push_back(record);
  if (undo_stack_.size() > max_edit_history_records) {
    undo_stack_.erase(undo_stack_.begin());
  }
}

void TextModel::commit_history_record(TextHistorySnapshot before) {
  TextEditHistoryRecord record{
      .before = std::move(before),
      .after = history_snapshot(),
  };
  if (history_snapshots_equal(record.before, record.after)) {
    return;
  }
  push_undo_record(record);
  redo_stack_.clear();
}

} // namespace cgpui
