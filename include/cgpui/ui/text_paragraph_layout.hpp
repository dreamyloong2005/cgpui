#pragma once

#include "cgpui/ui/text_measurement.hpp"
#include "cgpui/ui/text_wrapping.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct TextParagraphLayoutKey {
  std::string text;
  FontDescriptor font;
  float font_size = 16.0F;
  float scale = 1.0F;
  float max_width = 0.0F;

  friend bool operator==(
      const TextParagraphLayoutKey&,
      const TextParagraphLayoutKey&) = default;
};

struct TextParagraphLayout {
  TextMeasurement measurement;
  TextWrapLayout wrap_layout;
};

struct TextParagraphLayoutResult {
  TextParagraphLayout layout;
  bool cache_hit = false;
};

[[nodiscard]] TextParagraphLayout layout_text_paragraph(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F,
    DpiScale scale = {},
    float max_width = 0.0F);

class TextParagraphLayoutCache {
 public:
  [[nodiscard]] TextParagraphLayoutResult layout(
      std::string_view text,
      FontDescriptor font = {},
      float font_size = 16.0F,
      DpiScale scale = {},
      float max_width = 0.0F);

  [[nodiscard]] std::size_t entry_count() const;
  [[nodiscard]] std::size_t lookup_count() const;
  [[nodiscard]] std::size_t hit_count() const;
  [[nodiscard]] std::size_t miss_count() const;

  void clear();

 private:
  struct Entry {
    TextParagraphLayoutKey key;
    TextParagraphLayout layout;
  };

  std::vector<Entry> entries_;
  std::size_t lookup_count_ = 0;
  std::size_t hit_count_ = 0;
  std::size_t miss_count_ = 0;
};

} // namespace cgpui
