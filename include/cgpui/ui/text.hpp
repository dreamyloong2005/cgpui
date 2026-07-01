#pragma once

#include <cstddef>
#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cgpui/ui/style.hpp"

namespace cgpui {

enum class FontSource {
  platform,
  test,
};

struct FontFaceDescriptor {
  FontDescriptor font;
  std::string postscript_name;
  FontSource source = FontSource::platform;
  std::string path;

  friend bool operator==(
      const FontFaceDescriptor&,
      const FontFaceDescriptor&) = default;
};

class FontDatabase {
 public:
  [[nodiscard]] bool empty() const {
    return faces_.empty();
  }

  [[nodiscard]] std::size_t face_count() const {
    return faces_.size();
  }

  [[nodiscard]] std::span<const FontFaceDescriptor> faces() const {
    return faces_;
  }

  void add_face(FontFaceDescriptor face) {
    if (face.font.family.empty()) {
      return;
    }
    for (const auto& existing : faces_) {
      if (existing == face) {
        return;
      }
    }
    faces_.push_back(std::move(face));
  }

  [[nodiscard]] const FontFaceDescriptor* resolve(
      const FontDescriptor& descriptor) const {
    if (descriptor.family.empty()) {
      return faces_.empty() ? nullptr : &faces_.front();
    }
    for (const auto& face : faces_) {
      if (face.font.family == descriptor.family) {
        return &face;
      }
    }
    return nullptr;
  }

 private:
  std::vector<FontFaceDescriptor> faces_;
};

[[nodiscard]] inline FontDatabase discover_test_fonts(
    std::span<const FontFaceDescriptor> faces) {
  FontDatabase database;
  for (const auto& face : faces) {
    database.add_face(face);
  }
  return database;
}

struct TextGlyphRun {
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;
  float advance = 0.0F;
};

struct TextShapeRun {
  std::string text;
  FontDescriptor font;
  float font_size = 16.0F;
  std::vector<TextGlyphRun> glyphs;
  std::size_t byte_length = 0;
  float total_advance = 0.0F;
  float line_height = 16.0F;

  [[nodiscard]] std::size_t glyph_count() const {
    return glyphs.size();
  }
};

struct GlyphAtlasKey {
  std::string font_family;
  float font_size = 16.0F;
  std::size_t glyph_index = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;

  friend bool operator==(const GlyphAtlasKey&, const GlyphAtlasKey&) = default;
};

struct TextGlyphPaint {
  GlyphAtlasKey key;
  Point origin;
  float advance = 0.0F;
};

[[nodiscard]] inline bool is_utf8_continuation_byte(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

[[nodiscard]] inline TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F) {
  TextShapeRun run{
      .text = std::string(text),
      .font = std::move(font),
      .font_size = font_size,
      .byte_length = text.size(),
      .line_height = font_size,
  };
  const float fallback_advance = font_size * 0.5F;
  std::size_t byte_offset = 0;
  while (byte_offset < text.size()) {
    std::size_t byte_length = 1;
    while (byte_offset + byte_length < text.size() &&
           is_utf8_continuation_byte(text[byte_offset + byte_length])) {
      byte_length += 1;
    }
    run.glyphs.push_back(TextGlyphRun{
        .byte_offset = byte_offset,
        .byte_length = byte_length,
        .advance = fallback_advance,
    });
    run.total_advance += fallback_advance;
    byte_offset += byte_length;
  }
  return run;
}

[[nodiscard]] inline std::vector<TextGlyphPaint> text_glyph_paint_metadata(
    const TextShapeRun& run,
    Point origin = {}) {
  std::vector<TextGlyphPaint> glyphs;
  glyphs.reserve(run.glyphs.size());
  float x = origin.x;
  for (std::size_t index = 0; index < run.glyphs.size(); ++index) {
    const TextGlyphRun& glyph = run.glyphs[index];
    glyphs.push_back(TextGlyphPaint{
        .key =
            GlyphAtlasKey{
                .font_family = run.font.family,
                .font_size = run.font_size,
                .glyph_index = index,
                .byte_offset = glyph.byte_offset,
                .byte_length = glyph.byte_length,
            },
        .origin = Point{.x = x, .y = origin.y},
        .advance = glyph.advance,
    });
    x += glyph.advance;
  }
  return glyphs;
}

struct TextSelectionRange {
  std::size_t start = 0;
  std::size_t end = 0;
  bool collapsed = true;
};

enum class TextEditAction {
  move_previous,
  move_next,
  extend_previous,
  extend_next,
  backspace,
  delete_forward,
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

  [[nodiscard]] bool has_composition() const {
    return has_composition_;
  }

  [[nodiscard]] std::string_view composition_text() const {
    return composition_text_;
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

  [[nodiscard]] std::string selected_text() const {
    const TextSelectionRange range = selection();
    if (range.collapsed) {
      return {};
    }
    return text_.substr(range.start, range.end - range.start);
  }

  void set_selection(std::size_t anchor, std::size_t head) {
    selection_anchor_ = clamp_offset(anchor);
    selection_head_ = clamp_offset(head);
    cursor_ = selection_head_;
  }

  void clear_selection() {
    collapse_selection_to_cursor();
  }

  void set_composition_text(std::string_view text) {
    composition_text_ = std::string(text);
    has_composition_ = true;
  }

  void commit_composition() {
    if (!has_composition_) {
      return;
    }
    std::string committed = std::move(composition_text_);
    clear_composition();
    insert_text(committed);
  }

  void cancel_composition() {
    clear_composition();
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
    if (erase_selection_if_needed()) {
      return true;
    }
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
    if (erase_selection_if_needed()) {
      return true;
    }
    if (cursor_ >= text_.size()) {
      return false;
    }
    const std::size_t next = next_codepoint_boundary(cursor_);
    text_.erase(cursor_, next - cursor_);
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool apply_edit_action(TextEditAction action) {
    switch (action) {
      case TextEditAction::move_previous:
        return move_cursor_previous();
      case TextEditAction::move_next:
        return move_cursor_next();
      case TextEditAction::extend_previous:
        return extend_selection_previous();
      case TextEditAction::extend_next:
        return extend_selection_next();
      case TextEditAction::backspace:
        return backspace();
      case TextEditAction::delete_forward:
        return delete_forward();
    }
    return false;
  }

 private:
  void clear_composition() {
    composition_text_.clear();
    has_composition_ = false;
  }

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

  [[nodiscard]] bool extend_selection_previous() {
    if (selection().collapsed) {
      selection_anchor_ = cursor_;
    }
    if (cursor_ == 0) {
      return false;
    }
    cursor_ = previous_codepoint_boundary(cursor_);
    selection_head_ = cursor_;
    return true;
  }

  [[nodiscard]] bool extend_selection_next() {
    if (selection().collapsed) {
      selection_anchor_ = cursor_;
    }
    if (cursor_ >= text_.size()) {
      return false;
    }
    cursor_ = next_codepoint_boundary(cursor_);
    selection_head_ = cursor_;
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
  std::string composition_text_;
  std::size_t cursor_ = 0;
  std::size_t selection_anchor_ = 0;
  std::size_t selection_head_ = 0;
  bool has_composition_ = false;
};

} // namespace cgpui
