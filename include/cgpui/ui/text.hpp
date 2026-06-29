#pragma once

#include <cstddef>
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

struct TextSelectionRange {
  std::size_t start = 0;
  std::size_t end = 0;
  bool collapsed = true;
};

class TextModel {
 public:
  TextModel() = default;

  explicit TextModel(std::string text) : text_(std::move(text)) {
    cursor_ = text_.size();
    collapse_selection_to_cursor();
  }

  [[nodiscard]] std::string_view text() const {
    return text_;
  }

  [[nodiscard]] std::size_t cursor() const {
    return cursor_;
  }

  [[nodiscard]] std::size_t selection_anchor() const {
    return selection_anchor_;
  }

  [[nodiscard]] std::size_t selection_head() const {
    return selection_head_;
  }

  [[nodiscard]] TextSelectionRange selection() const {
    const std::size_t start = std::min(selection_anchor_, selection_head_);
    const std::size_t end = std::max(selection_anchor_, selection_head_);
    return TextSelectionRange{
        .start = start,
        .end = end,
        .collapsed = start == end,
    };
  }

  void set_selection(std::size_t anchor, std::size_t head) {
    selection_anchor_ = clamp_offset(anchor);
    selection_head_ = clamp_offset(head);
    cursor_ = selection_head_;
  }

  void clear_selection() {
    collapse_selection_to_cursor();
  }

  void insert_text(std::string_view text) {
    (void)erase_selection_if_needed();
    text_.insert(cursor_, text);
    cursor_ += text.size();
    collapse_selection_to_cursor();
  }

  [[nodiscard]] bool move_cursor_previous() {
    if (cursor_ == 0) {
      return false;
    }
    cursor_ = previous_codepoint_boundary(cursor_);
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool move_cursor_next() {
    if (cursor_ >= text_.size()) {
      return false;
    }
    cursor_ = next_codepoint_boundary(cursor_);
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool backspace() {
    if (cursor_ == 0) {
      return false;
    }
    const std::size_t previous = previous_codepoint_boundary(cursor_);
    text_.erase(previous, cursor_ - previous);
    cursor_ = previous;
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool delete_forward() {
    if (cursor_ >= text_.size()) {
      return false;
    }
    const std::size_t next = next_codepoint_boundary(cursor_);
    text_.erase(cursor_, next - cursor_);
    collapse_selection_to_cursor();
    return true;
  }

 private:
  [[nodiscard]] std::size_t clamp_offset(std::size_t offset) const {
    return std::min(offset, text_.size());
  }

  void collapse_selection_to_cursor() {
    selection_anchor_ = cursor_;
    selection_head_ = cursor_;
  }

  [[nodiscard]] bool erase_selection_if_needed() {
    const TextSelectionRange range = selection();
    if (range.collapsed) {
      return false;
    }
    text_.erase(range.start, range.end - range.start);
    cursor_ = range.start;
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] static bool is_utf8_continuation(char value) {
    return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
  }

  [[nodiscard]] std::size_t previous_codepoint_boundary(
      std::size_t offset) const {
    std::size_t index = offset - 1;
    while (index > 0 && is_utf8_continuation(text_[index])) {
      index -= 1;
    }
    return index;
  }

  [[nodiscard]] std::size_t next_codepoint_boundary(std::size_t offset) const {
    std::size_t index = offset + 1;
    while (index < text_.size() && is_utf8_continuation(text_[index])) {
      index += 1;
    }
    return index;
  }

  std::string text_;
  std::size_t cursor_ = 0;
  std::size_t selection_anchor_ = 0;
  std::size_t selection_head_ = 0;
};

} // namespace cgpui
