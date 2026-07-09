#pragma once

#include "cgpui/ui/text_edit_actions.hpp"
#include "cgpui/ui/text_hit_testing.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

enum class TextInsertHistoryPolicy : std::uint8_t {
  merge_adjacent_typing,
  composition_commit,
  separate_edit,
};

enum class TextEditHistoryRedoInvalidationReason : std::uint8_t {
  none,
  branch_edit,
};

struct TextEditHistoryRedoInvalidation {
  TextEditHistoryRedoInvalidationReason reason =
      TextEditHistoryRedoInvalidationReason::none;
  std::size_t cleared_redo_depth = 0;
  std::uint64_t revision = 0;
};

struct TextEditHistoryStatus {
  bool can_undo = false;
  bool can_redo = false;
  bool clean = true;
  std::size_t undo_depth = 0;
  std::size_t redo_depth = 0;
  std::uint64_t revision = 0;
  TextEditHistoryRedoInvalidation last_redo_invalidation;
};

class TextModel {
 public:
  TextModel() = default;
  explicit TextModel(std::string text);

  [[nodiscard]] std::string_view text() const;
  [[nodiscard]] std::size_t cursor() const;
  [[nodiscard]] std::size_t selection_anchor() const;
  [[nodiscard]] std::size_t selection_head() const;
  [[nodiscard]] bool has_composition() const;
  [[nodiscard]] std::string_view composition_text() const;
  [[nodiscard]] TextSelectionRange selection() const;
  [[nodiscard]] std::string selected_text() const;
  [[nodiscard]] std::size_t line_count() const;
  [[nodiscard]] std::size_t line_index_at(std::size_t offset) const;
  [[nodiscard]] std::size_t line_start_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t line_end_offset(std::size_t offset) const;
  [[nodiscard]] TextSelectionRange word_selection_range_at(
      std::size_t offset) const;
  [[nodiscard]] TextSelectionRange line_selection_range_at(
      std::size_t offset) const;

  void set_selection(std::size_t anchor, std::size_t head);
  void clear_selection();
  void set_composition_text(std::string_view text);
  void commit_composition();
  void cancel_composition();
  void insert_text(std::string_view text);
  void insert_text(
      std::string_view text,
      TextInsertHistoryPolicy history_policy);

  [[nodiscard]] bool move_cursor_previous();
  [[nodiscard]] bool move_cursor_next();
  [[nodiscard]] bool move_cursor_previous_word();
  [[nodiscard]] bool move_cursor_next_word();
  [[nodiscard]] bool move_cursor_line_start();
  [[nodiscard]] bool move_cursor_line_end();
  [[nodiscard]] bool move_cursor_previous_line();
  [[nodiscard]] bool move_cursor_next_line();
  [[nodiscard]] bool can_undo() const;
  [[nodiscard]] bool can_redo() const;
  [[nodiscard]] bool edit_history_clean() const;
  [[nodiscard]] TextEditHistoryStatus edit_history_status() const;
  void mark_edit_history_clean();
  [[nodiscard]] bool undo();
  [[nodiscard]] bool redo();
  [[nodiscard]] bool backspace();
  [[nodiscard]] bool delete_forward();
  [[nodiscard]] bool delete_surrounding_text(
      std::size_t before_length,
      std::size_t after_length);
  [[nodiscard]] bool apply_edit_action(TextEditAction action);

 private:
  struct TextHistorySnapshot {
    std::string text;
    std::size_t cursor = 0;
    std::size_t selection_anchor = 0;
    std::size_t selection_head = 0;
  };

  struct TextEditHistoryRecord {
    TextHistorySnapshot before;
    TextHistorySnapshot after;
    TextInsertHistoryPolicy history_policy =
        TextInsertHistoryPolicy::separate_edit;
  };

  struct DecodedCodepoint {
    std::uint32_t value = 0;
    std::size_t next = 0;
  };

  void clear_composition();
  void begin_composition_history_group();
  void clear_composition_history_group();
  void clear_preferred_line_column();
  void clear_edit_history_grouping();
  [[nodiscard]] std::size_t preferred_line_column_for_vertical_navigation();
  [[nodiscard]] TextHistorySnapshot history_snapshot() const;
  [[nodiscard]] static bool history_snapshots_equal(
      const TextHistorySnapshot& left,
      const TextHistorySnapshot& right);
  void restore_history_snapshot(const TextHistorySnapshot& snapshot);
  void push_undo_record(const TextEditHistoryRecord& record);
  void invalidate_redo_history(
      TextEditHistoryRedoInvalidationReason reason);
  void commit_history_record(
      TextHistorySnapshot before,
      TextInsertHistoryPolicy history_policy =
          TextInsertHistoryPolicy::separate_edit);
  [[nodiscard]] std::size_t clamp_offset(std::size_t offset) const;
  void collapse_selection_to_cursor();
  [[nodiscard]] bool move_cursor_to(std::size_t offset);
  [[nodiscard]] bool extend_selection_to(std::size_t offset);
  [[nodiscard]] bool erase_selection_if_needed();
  [[nodiscard]] bool extend_selection_previous();
  [[nodiscard]] bool extend_selection_next();
  [[nodiscard]] bool extend_selection_previous_word();
  [[nodiscard]] bool extend_selection_next_word();
  [[nodiscard]] bool extend_selection_line_start();
  [[nodiscard]] bool extend_selection_line_end();
  [[nodiscard]] bool extend_selection_previous_line();
  [[nodiscard]] bool extend_selection_next_line();
  [[nodiscard]] static bool is_utf8_continuation(char value);
  [[nodiscard]] std::size_t line_start_for_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t line_end_for_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t line_column_for_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t previous_line_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t next_line_offset(std::size_t offset) const;
  [[nodiscard]] std::size_t previous_codepoint_boundary(
      std::size_t offset) const;
  [[nodiscard]] std::size_t next_codepoint_boundary(std::size_t offset) const;
  [[nodiscard]] DecodedCodepoint codepoint_at(std::size_t offset) const;
  [[nodiscard]] static bool is_combining_mark(std::uint32_t value);
  [[nodiscard]] static bool is_variation_selector(std::uint32_t value);
  [[nodiscard]] static bool is_regional_indicator(std::uint32_t value);
  [[nodiscard]] static bool is_zero_width_joiner(std::uint32_t value);
  [[nodiscard]] static bool is_unicode_space(std::uint32_t value);
  [[nodiscard]] bool grapheme_at_is_word_separator(
      std::size_t offset) const;
  [[nodiscard]] std::size_t next_grapheme_boundary(std::size_t offset) const;
  [[nodiscard]] std::size_t previous_grapheme_boundary(
      std::size_t offset) const;
  [[nodiscard]] std::size_t previous_word_boundary(std::size_t offset) const;
  [[nodiscard]] std::size_t next_word_boundary(std::size_t offset) const;

  std::string text_;
  std::string composition_text_;
  std::optional<TextHistorySnapshot> composition_history_before_;
  std::size_t cursor_ = 0;
  std::size_t selection_anchor_ = 0;
  std::size_t selection_head_ = 0;
  std::optional<std::size_t> preferred_line_column_;
  bool has_composition_ = false;
  bool edit_history_grouping_open_ = false;
  bool composition_history_mutated_ = false;
  bool clean_edit_history_marker_valid_ = true;
  std::size_t clean_edit_history_undo_depth_ = 0;
  std::uint64_t edit_history_revision_ = 0;
  TextEditHistoryRedoInvalidation last_redo_invalidation_;
  std::vector<TextEditHistoryRecord> undo_stack_;
  std::vector<TextEditHistoryRecord> redo_stack_;
  static constexpr std::size_t max_edit_history_records = 100;
};

} // namespace cgpui
