#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "cgpui/ui/layout.hpp"
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

class FontFallbackChain {
 public:
  [[nodiscard]] bool empty() const {
    return faces_.empty();
  }

  [[nodiscard]] std::size_t size() const {
    return faces_.size();
  }

  [[nodiscard]] std::span<const FontFaceDescriptor* const> faces() const {
    return faces_;
  }

  [[nodiscard]] const FontFaceDescriptor* primary() const {
    return faces_.empty() ? nullptr : faces_.front();
  }

  void add_face(const FontFaceDescriptor* face) {
    if (face == nullptr || contains(face)) {
      return;
    }
    faces_.push_back(face);
  }

 private:
  [[nodiscard]] bool contains(const FontFaceDescriptor* face) const {
    return std::find(faces_.begin(), faces_.end(), face) != faces_.end();
  }

  std::vector<const FontFaceDescriptor*> faces_;
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

  void add_generic_fallback_family(std::string_view family) {
    if (family.empty()) {
      return;
    }
    if (std::find(
            generic_fallback_families_.begin(),
            generic_fallback_families_.end(),
            family) != generic_fallback_families_.end()) {
      return;
    }
    generic_fallback_families_.push_back(std::string(family));
  }

  [[nodiscard]] std::span<const std::string> generic_fallback_families()
      const {
    return generic_fallback_families_;
  }

  [[nodiscard]] const FontFaceDescriptor* resolve(
      const FontDescriptor& descriptor) const {
    if (descriptor.family.empty()) {
      return faces_.empty() ? nullptr : &faces_.front();
    }
    return find_face_for_family(descriptor.family);
  }

  [[nodiscard]] FontFallbackChain resolve_chain(
      const FontDescriptor& descriptor) const {
    FontFallbackChain chain;
    if (faces_.empty()) {
      return chain;
    }
    if (descriptor.family.empty()) {
      chain.add_face(&faces_.front());
      return chain;
    }

    chain.add_face(find_face_for_family(descriptor.family));
    for (const auto& fallback_family : generic_fallback_families_) {
      chain.add_face(find_face_for_family(fallback_family));
    }
    chain.add_face(&faces_.front());
    return chain;
  }

 private:
  [[nodiscard]] const FontFaceDescriptor* find_face_for_family(
      std::string_view family) const {
    for (const auto& face : faces_) {
      if (face.font.family == family) {
        return &face;
      }
    }
    return nullptr;
  }

  std::vector<FontFaceDescriptor> faces_;
  std::vector<std::string> generic_fallback_families_;
};

[[nodiscard]] inline FontDatabase font_database_from_discovered_faces(
    std::span<const FontFaceDescriptor> faces) {
  FontDatabase database;
  for (const auto& face : faces) {
    database.add_face(face);
  }
  return database;
}

[[nodiscard]] inline FontDatabase discover_test_fonts(
    std::span<const FontFaceDescriptor> faces) {
  return font_database_from_discovered_faces(faces);
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
  DpiScale scale;
  std::vector<TextGlyphRun> glyphs;
  std::size_t byte_length = 0;
  float total_advance = 0.0F;
  float line_height = 16.0F;
  float device_font_size = 16.0F;
  float device_total_advance = 0.0F;
  float device_line_height = 16.0F;

  [[nodiscard]] std::size_t glyph_count() const {
    return glyphs.size();
  }
};

struct GlyphAtlasKey {
  std::string font_family;
  float font_size = 16.0F;
  float scale = 1.0F;
  float device_font_size = 16.0F;
  std::size_t glyph_index = 0;
  std::size_t byte_offset = 0;
  std::size_t byte_length = 0;

  friend bool operator==(const GlyphAtlasKey&, const GlyphAtlasKey&) = default;
};

struct TextGlyphPaint {
  GlyphAtlasKey key;
  Point origin;
  float advance = 0.0F;
  Point device_origin;
  float device_advance = 0.0F;
};

struct GlyphBitmap {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;
  std::vector<std::uint8_t> alpha;

  [[nodiscard]] bool empty() const {
    return width == 0 || height == 0 || stride == 0 || alpha.empty();
  }

  [[nodiscard]] std::size_t byte_size() const {
    return alpha.size();
  }

  [[nodiscard]] std::uint8_t pixel(
      std::uint32_t x,
      std::uint32_t y) const {
    if (x >= width || y >= height || stride == 0) {
      return 0;
    }
    const std::size_t index =
        static_cast<std::size_t>(y) * static_cast<std::size_t>(stride) + x;
    return index < alpha.size() ? alpha[index] : 0;
  }
};

struct GlyphRasterizerOptions {
  std::uint8_t foreground_alpha = 255;
  std::uint8_t background_alpha = 0;
  std::uint32_t padding = 1;
};

struct RasterizedGlyph {
  GlyphAtlasKey key;
  GlyphBitmap bitmap;
  float advance = 0.0F;
  float device_font_size = 16.0F;
  float left_bearing = 0.0F;
  float top_bearing = 0.0F;
  float baseline = 0.0F;
};

[[nodiscard]] inline std::uint32_t rasterized_glyph_dimension(float value) {
  if (value <= 0.0F) {
    return 1;
  }
  return std::max(1U, static_cast<std::uint32_t>(value + 0.5F));
}

[[nodiscard]] inline RasterizedGlyph rasterize_fallback_glyph(
    const TextGlyphPaint& glyph,
    GlyphRasterizerOptions options = {}) {
  const std::uint32_t width =
      rasterized_glyph_dimension(glyph.device_advance);
  const std::uint32_t height =
      rasterized_glyph_dimension(glyph.key.device_font_size);
  const std::uint32_t stride = width;
  std::vector<std::uint8_t> alpha(
      static_cast<std::size_t>(stride) * static_cast<std::size_t>(height),
      options.background_alpha);

  const bool can_pad =
      width > options.padding * 2U && height > options.padding * 2U;
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const bool inside = !can_pad ||
          (x >= options.padding && x + options.padding < width &&
           y >= options.padding && y + options.padding < height);
      if (inside) {
        alpha[static_cast<std::size_t>(y) * stride + x] =
            options.foreground_alpha;
      }
    }
  }

  return RasterizedGlyph{
      .key = glyph.key,
      .bitmap =
          GlyphBitmap{
              .width = width,
              .height = height,
              .stride = stride,
              .alpha = std::move(alpha),
          },
      .advance = glyph.device_advance,
      .device_font_size = glyph.key.device_font_size,
      .left_bearing = 0.0F,
      .top_bearing = 0.0F,
      .baseline = glyph.key.device_font_size * 0.8F,
  };
}

[[nodiscard]] inline bool is_utf8_continuation_byte(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

[[nodiscard]] inline TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F,
    DpiScale scale = {}) {
  const float scale_value = normalized_scale(scale);
  TextShapeRun run{
      .text = std::string(text),
      .font = std::move(font),
      .font_size = font_size,
      .scale = scale,
      .byte_length = text.size(),
      .line_height = font_size,
      .device_font_size = font_size * scale_value,
      .device_line_height = font_size * scale_value,
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
    run.device_total_advance += fallback_advance * scale_value;
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
    const Point logical_origin{.x = x, .y = origin.y};
    glyphs.push_back(TextGlyphPaint{
        .key =
            GlyphAtlasKey{
                .font_family = run.font.family,
                .font_size = run.font_size,
                .scale = normalized_scale(run.scale),
                .device_font_size = run.device_font_size,
                .glyph_index = index,
                .byte_offset = glyph.byte_offset,
                .byte_length = glyph.byte_length,
            },
        .origin = logical_origin,
        .advance = glyph.advance,
        .device_origin = to_device_pixels(logical_origin, run.scale),
        .device_advance = glyph.advance * normalized_scale(run.scale),
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
  move_previous_word,
  move_next_word,
  extend_previous_word,
  extend_next_word,
  move_line_start,
  move_line_end,
  move_previous_line,
  move_next_line,
  extend_line_start,
  extend_line_end,
  extend_previous_line,
  extend_next_line,
  undo,
  redo,
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

  [[nodiscard]] std::size_t line_count() const {
    return static_cast<std::size_t>(
        std::count(text_.begin(), text_.end(), '\n')) + 1U;
  }

  [[nodiscard]] std::size_t line_index_at(std::size_t offset) const {
    const std::size_t clamped = clamp_offset(offset);
    return static_cast<std::size_t>(
        std::count(text_.begin(), text_.begin() + clamped, '\n'));
  }

  [[nodiscard]] std::size_t line_start_offset(std::size_t offset) const {
    return line_start_for_offset(offset);
  }

  [[nodiscard]] std::size_t line_end_offset(std::size_t offset) const {
    return line_end_for_offset(offset);
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
    const TextHistorySnapshot before = history_snapshot();
    (void)erase_selection_if_needed();
    text_.insert(cursor_, text);
    cursor_ += text.size();
    collapse_selection_to_cursor();
    commit_history_record(before);
  }

  [[nodiscard]] bool move_cursor_previous() {
    if (cursor_ == 0) {
      return false;
    }
    cursor_ = previous_grapheme_boundary(cursor_);
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool move_cursor_next() {
    if (cursor_ >= text_.size()) {
      return false;
    }
    cursor_ = next_grapheme_boundary(cursor_);
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool move_cursor_previous_word() {
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

  [[nodiscard]] bool move_cursor_next_word() {
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

  [[nodiscard]] bool move_cursor_line_start() {
    return move_cursor_to(line_start_for_offset(cursor_));
  }

  [[nodiscard]] bool move_cursor_line_end() {
    return move_cursor_to(line_end_for_offset(cursor_));
  }

  [[nodiscard]] bool move_cursor_previous_line() {
    const std::size_t target = previous_line_offset(cursor_);
    if (target == cursor_) {
      return false;
    }
    return move_cursor_to(target);
  }

  [[nodiscard]] bool move_cursor_next_line() {
    const std::size_t target = next_line_offset(cursor_);
    if (target == cursor_) {
      return false;
    }
    return move_cursor_to(target);
  }

  [[nodiscard]] bool can_undo() const {
    return !undo_stack_.empty();
  }

  [[nodiscard]] bool can_redo() const {
    return !redo_stack_.empty();
  }

  [[nodiscard]] bool undo() {
    if (undo_stack_.empty()) {
      return false;
    }
    const TextEditHistoryRecord record = undo_stack_.back();
    undo_stack_.pop_back();
    redo_stack_.push_back(record);
    restore_history_snapshot(record.before);
    return true;
  }

  [[nodiscard]] bool redo() {
    if (redo_stack_.empty()) {
      return false;
    }
    const TextEditHistoryRecord record = redo_stack_.back();
    redo_stack_.pop_back();
    push_undo_record(record);
    restore_history_snapshot(record.after);
    return true;
  }

  [[nodiscard]] bool backspace() {
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

  [[nodiscard]] bool delete_forward() {
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

  [[nodiscard]] bool delete_surrounding_text(
      std::size_t before_length,
      std::size_t after_length) {
    const TextHistorySnapshot before = history_snapshot();
    const std::size_t cursor = clamp_offset(cursor_);
    std::size_t start =
        before_length > cursor ? 0 : cursor - before_length;
    while (start > 0 && is_utf8_continuation(text_[start])) {
      start -= 1;
    }

    const std::size_t available_after = text_.size() - cursor;
    std::size_t end =
        cursor + std::min(after_length, available_after);
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

 private:
  void clear_composition() {
    composition_text_.clear();
    has_composition_ = false;
  }

  struct TextHistorySnapshot {
    std::string text;
    std::size_t cursor = 0;
    std::size_t selection_anchor = 0;
    std::size_t selection_head = 0;
  };

  struct TextEditHistoryRecord {
    TextHistorySnapshot before;
    TextHistorySnapshot after;
  };

  [[nodiscard]] TextHistorySnapshot history_snapshot() const {
    return TextHistorySnapshot{
        .text = text_,
        .cursor = cursor_,
        .selection_anchor = selection_anchor_,
        .selection_head = selection_head_,
    };
  }

  [[nodiscard]] static bool history_snapshots_equal(
      const TextHistorySnapshot& left,
      const TextHistorySnapshot& right) {
    return left.text == right.text && left.cursor == right.cursor &&
        left.selection_anchor == right.selection_anchor &&
        left.selection_head == right.selection_head;
  }

  void restore_history_snapshot(const TextHistorySnapshot& snapshot) {
    text_ = snapshot.text;
    cursor_ = std::min(snapshot.cursor, text_.size());
    selection_anchor_ = std::min(snapshot.selection_anchor, text_.size());
    selection_head_ = std::min(snapshot.selection_head, text_.size());
    clear_composition();
  }

  void push_undo_record(const TextEditHistoryRecord& record) {
    undo_stack_.push_back(record);
    if (undo_stack_.size() > max_edit_history_records) {
      undo_stack_.erase(undo_stack_.begin());
    }
  }

  void commit_history_record(TextHistorySnapshot before) {
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

  [[nodiscard]] std::size_t clamp_offset(std::size_t offset) const {
    return std::min(offset, text_.size());
  }

  void collapse_selection_to_cursor() {
    selection_anchor_ = cursor_;
    selection_head_ = cursor_;
  }

  [[nodiscard]] bool move_cursor_to(std::size_t offset) {
    const std::size_t target = clamp_offset(offset);
    if (target == cursor_) {
      return false;
    }
    cursor_ = target;
    collapse_selection_to_cursor();
    return true;
  }

  [[nodiscard]] bool extend_selection_to(std::size_t offset) {
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
    cursor_ = previous_grapheme_boundary(cursor_);
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
    cursor_ = next_grapheme_boundary(cursor_);
    selection_head_ = cursor_;
    return true;
  }

  [[nodiscard]] bool extend_selection_previous_word() {
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

  [[nodiscard]] bool extend_selection_next_word() {
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

  [[nodiscard]] bool extend_selection_line_start() {
    return extend_selection_to(line_start_for_offset(cursor_));
  }

  [[nodiscard]] bool extend_selection_line_end() {
    return extend_selection_to(line_end_for_offset(cursor_));
  }

  [[nodiscard]] bool extend_selection_previous_line() {
    return extend_selection_to(previous_line_offset(cursor_));
  }

  [[nodiscard]] bool extend_selection_next_line() {
    return extend_selection_to(next_line_offset(cursor_));
  }

  [[nodiscard]] static bool is_utf8_continuation(char value) {
    return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
  }

  [[nodiscard]] std::size_t line_start_for_offset(std::size_t offset) const {
    const std::size_t clamped = clamp_offset(offset);
    if (clamped == 0 || text_.empty()) {
      return 0;
    }
    const std::size_t search_from = clamped - 1;
    const std::size_t newline = text_.rfind('\n', search_from);
    return newline == std::string::npos ? 0 : newline + 1;
  }

  [[nodiscard]] std::size_t line_end_for_offset(std::size_t offset) const {
    const std::size_t clamped = clamp_offset(offset);
    const std::size_t newline = text_.find('\n', clamped);
    return newline == std::string::npos ? text_.size() : newline;
  }

  [[nodiscard]] std::size_t line_column_for_offset(std::size_t offset) const {
    const std::size_t clamped = clamp_offset(offset);
    return clamped - line_start_for_offset(clamped);
  }

  [[nodiscard]] std::size_t previous_line_offset(std::size_t offset) const {
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

  [[nodiscard]] std::size_t next_line_offset(std::size_t offset) const {
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

  struct DecodedCodepoint {
    std::uint32_t value = 0;
    std::size_t next = 0;
  };

  [[nodiscard]] DecodedCodepoint codepoint_at(std::size_t offset) const {
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
          .value =
              ((static_cast<std::uint32_t>(first) & 0x1FU) << 6U) |
              (static_cast<std::uint32_t>(second) & 0x3FU),
          .next = next,
      };
    }
    if (next - offset == 3) {
      const auto second = static_cast<unsigned char>(text_[offset + 1]);
      const auto third = static_cast<unsigned char>(text_[offset + 2]);
      return DecodedCodepoint{
          .value =
              ((static_cast<std::uint32_t>(first) & 0x0FU) << 12U) |
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
          .value =
              ((static_cast<std::uint32_t>(first) & 0x07U) << 18U) |
              ((static_cast<std::uint32_t>(second) & 0x3FU) << 12U) |
              ((static_cast<std::uint32_t>(third) & 0x3FU) << 6U) |
              (static_cast<std::uint32_t>(fourth) & 0x3FU),
          .next = next,
      };
    }
    return DecodedCodepoint{.value = first, .next = next};
  }

  [[nodiscard]] static bool is_combining_mark(std::uint32_t value) {
    return (value >= 0x0300U && value <= 0x036FU) ||
        (value >= 0x1AB0U && value <= 0x1AFFU) ||
        (value >= 0x1DC0U && value <= 0x1DFFU) ||
        (value >= 0x20D0U && value <= 0x20FFU) ||
        (value >= 0xFE20U && value <= 0xFE2FU);
  }

  [[nodiscard]] static bool is_variation_selector(std::uint32_t value) {
    return (value >= 0xFE00U && value <= 0xFE0FU) ||
        (value >= 0xE0100U && value <= 0xE01EFU);
  }

  [[nodiscard]] static bool is_regional_indicator(std::uint32_t value) {
    return value >= 0x1F1E6U && value <= 0x1F1FFU;
  }

  [[nodiscard]] static bool is_zero_width_joiner(std::uint32_t value) {
    return value == 0x200DU;
  }

  [[nodiscard]] static bool is_unicode_space(std::uint32_t value) {
    return (value >= 0x0009U && value <= 0x000DU) || value == 0x0020U ||
        value == 0x0085U || value == 0x00A0U || value == 0x1680U ||
        (value >= 0x2000U && value <= 0x200AU) || value == 0x2028U ||
        value == 0x2029U || value == 0x202FU || value == 0x205FU ||
        value == 0x3000U;
  }

  [[nodiscard]] bool grapheme_at_is_word_separator(
      std::size_t offset) const {
    return offset >= text_.size() ||
        is_unicode_space(codepoint_at(offset).value);
  }

  [[nodiscard]] std::size_t next_grapheme_boundary(std::size_t offset) const {
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

  [[nodiscard]] std::size_t previous_grapheme_boundary(
      std::size_t offset) const {
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

  [[nodiscard]] std::size_t previous_word_boundary(std::size_t offset) const {
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

  [[nodiscard]] std::size_t next_word_boundary(std::size_t offset) const {
    std::size_t boundary = clamp_offset(offset);
    while (boundary < text_.size() && grapheme_at_is_word_separator(boundary)) {
      boundary = next_grapheme_boundary(boundary);
    }
    while (boundary < text_.size() && !grapheme_at_is_word_separator(boundary)) {
      boundary = next_grapheme_boundary(boundary);
    }
    return boundary;
  }

  std::string text_;
  std::string composition_text_;
  std::size_t cursor_ = 0;
  std::size_t selection_anchor_ = 0;
  std::size_t selection_head_ = 0;
  bool has_composition_ = false;
  std::vector<TextEditHistoryRecord> undo_stack_;
  std::vector<TextEditHistoryRecord> redo_stack_;
  static constexpr std::size_t max_edit_history_records = 100;
};

} // namespace cgpui
