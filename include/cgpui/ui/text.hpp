#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {

class TextModel {
 public:
  TextModel() = default;

  explicit TextModel(std::string text) : text_(std::move(text)) {
    cursor_ = text_.size();
  }

  [[nodiscard]] std::string_view text() const {
    return text_;
  }

  [[nodiscard]] std::size_t cursor() const {
    return cursor_;
  }

  void insert_text(std::string_view text) {
    text_.insert(cursor_, text);
    cursor_ += text.size();
  }

  [[nodiscard]] bool move_cursor_previous() {
    if (cursor_ == 0) {
      return false;
    }
    cursor_ = previous_codepoint_boundary(cursor_);
    return true;
  }

  [[nodiscard]] bool move_cursor_next() {
    if (cursor_ >= text_.size()) {
      return false;
    }
    cursor_ = next_codepoint_boundary(cursor_);
    return true;
  }

  [[nodiscard]] bool backspace() {
    if (cursor_ == 0) {
      return false;
    }
    const std::size_t previous = previous_codepoint_boundary(cursor_);
    text_.erase(previous, cursor_ - previous);
    cursor_ = previous;
    return true;
  }

  [[nodiscard]] bool delete_forward() {
    if (cursor_ >= text_.size()) {
      return false;
    }
    const std::size_t next = next_codepoint_boundary(cursor_);
    text_.erase(cursor_, next - cursor_);
    return true;
  }

 private:
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
};

} // namespace cgpui
