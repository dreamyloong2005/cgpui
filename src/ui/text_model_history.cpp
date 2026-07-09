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

bool TextModel::edit_history_clean() const {
  return clean_edit_history_marker_valid_ &&
      undo_stack_.size() == clean_edit_history_undo_depth_;
}

TextEditHistoryStatus TextModel::edit_history_status() const {
  return TextEditHistoryStatus{
      .can_undo = can_undo(),
      .can_redo = can_redo(),
      .clean = edit_history_clean(),
      .undo_depth = undo_stack_.size(),
      .redo_depth = redo_stack_.size(),
      .revision = edit_history_revision_,
      .last_redo_invalidation = last_redo_invalidation_,
      .last_transaction = last_transaction_,
  };
}

void TextModel::mark_edit_history_clean() {
  const bool was_clean = edit_history_clean();
  const std::size_t undo_depth_before = undo_stack_.size();
  const std::size_t redo_depth_before = redo_stack_.size();
  clean_edit_history_marker_valid_ = true;
  clean_edit_history_undo_depth_ = undo_stack_.size();
  clear_edit_history_grouping();
  if (!was_clean) {
    ++edit_history_revision_;
    record_edit_history_transaction(
        TextEditHistoryTransactionKind::clean_marked,
        TextInsertHistoryPolicy::separate_edit,
        undo_depth_before,
        redo_depth_before);
  }
}

bool TextModel::undo() {
  if (undo_stack_.empty()) {
    return false;
  }
  const std::size_t undo_depth_before = undo_stack_.size();
  const std::size_t redo_depth_before = redo_stack_.size();
  clear_edit_history_grouping();
  const TextEditHistoryRecord record = undo_stack_.back();
  undo_stack_.pop_back();
  redo_stack_.push_back(record);
  restore_history_snapshot(record.before);
  ++edit_history_revision_;
  record_edit_history_transaction(
      TextEditHistoryTransactionKind::undo_applied,
      record.history_policy,
      undo_depth_before,
      redo_depth_before);
  return true;
}

bool TextModel::redo() {
  if (redo_stack_.empty()) {
    return false;
  }
  const std::size_t undo_depth_before = undo_stack_.size();
  const std::size_t redo_depth_before = redo_stack_.size();
  clear_edit_history_grouping();
  const TextEditHistoryRecord record = redo_stack_.back();
  redo_stack_.pop_back();
  push_undo_record(record);
  restore_history_snapshot(record.after);
  ++edit_history_revision_;
  record_edit_history_transaction(
      TextEditHistoryTransactionKind::redo_applied,
      record.history_policy,
      undo_depth_before,
      redo_depth_before);
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
  clear_edit_history_grouping();
  clear_composition();
  clear_composition_history_group();
}

void TextModel::push_undo_record(const TextEditHistoryRecord& record) {
  undo_stack_.push_back(record);
  if (undo_stack_.size() > max_edit_history_records) {
    undo_stack_.erase(undo_stack_.begin());
    if (clean_edit_history_marker_valid_) {
      if (clean_edit_history_undo_depth_ == 0) {
        clean_edit_history_marker_valid_ = false;
      } else {
        --clean_edit_history_undo_depth_;
      }
    }
  }
}

void TextModel::invalidate_redo_history(
    TextEditHistoryRedoInvalidationReason reason) {
  if (redo_stack_.empty()) {
    return;
  }
  last_redo_invalidation_ = TextEditHistoryRedoInvalidation{
      .reason = reason,
      .cleared_redo_depth = redo_stack_.size(),
      .revision = edit_history_revision_ + 1,
  };
  redo_stack_.clear();
}

void TextModel::record_edit_history_transaction(
    TextEditHistoryTransactionKind kind,
    TextInsertHistoryPolicy history_policy,
    std::size_t undo_depth_before,
    std::size_t redo_depth_before) {
  last_transaction_ = TextEditHistoryTransactionDiagnostic{
      .kind = kind,
      .history_policy = history_policy,
      .undo_depth_before = undo_depth_before,
      .undo_depth_after = undo_stack_.size(),
      .redo_depth_before = redo_depth_before,
      .redo_depth_after = redo_stack_.size(),
      .revision = edit_history_revision_,
  };
}

void TextModel::commit_history_record(
    TextHistorySnapshot before,
    TextInsertHistoryPolicy history_policy) {
  const std::size_t before_undo_depth = undo_stack_.size();
  const std::size_t before_redo_depth = redo_stack_.size();
  TextEditHistoryRecord record{
      .before = std::move(before),
      .after = history_snapshot(),
      .history_policy = history_policy,
  };
  if (history_snapshots_equal(record.before, record.after)) {
    return;
  }
  if (history_policy == TextInsertHistoryPolicy::merge_adjacent_typing &&
      edit_history_grouping_open_ && !undo_stack_.empty()) {
    TextEditHistoryRecord& previous = undo_stack_.back();
    if (previous.history_policy ==
            TextInsertHistoryPolicy::merge_adjacent_typing &&
        history_snapshots_equal(previous.after, record.before)) {
      if (clean_edit_history_marker_valid_ &&
          clean_edit_history_undo_depth_ == undo_stack_.size()) {
        clean_edit_history_marker_valid_ = false;
      }
      previous.after = std::move(record.after);
      invalidate_redo_history(
          TextEditHistoryRedoInvalidationReason::branch_edit);
      edit_history_grouping_open_ = true;
      ++edit_history_revision_;
      record_edit_history_transaction(
          TextEditHistoryTransactionKind::record_merged,
          history_policy,
          before_undo_depth,
          before_redo_depth);
      return;
    }
  }
  if (!redo_stack_.empty() && clean_edit_history_marker_valid_ &&
      clean_edit_history_undo_depth_ > before_undo_depth) {
    clean_edit_history_marker_valid_ = false;
  }
  push_undo_record(record);
  invalidate_redo_history(
      TextEditHistoryRedoInvalidationReason::branch_edit);
  edit_history_grouping_open_ =
      history_policy == TextInsertHistoryPolicy::merge_adjacent_typing;
  ++edit_history_revision_;
  record_edit_history_transaction(
      TextEditHistoryTransactionKind::record_committed,
      history_policy,
      before_undo_depth,
      before_redo_depth);
}

} // namespace cgpui
