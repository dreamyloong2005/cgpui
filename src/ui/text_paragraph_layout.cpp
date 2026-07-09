#include "cgpui/ui/text_paragraph_layout.hpp"

#include <string>
#include <utility>

namespace cgpui {

TextParagraphLayout layout_text_paragraph(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale,
    float max_width) {
  TextMeasurement measurement =
      measure_text(text, std::move(font), font_size, scale);
  TextWrapLayout wrap_layout = wrap_text_measurement(measurement, max_width);
  return TextParagraphLayout{
      .measurement = std::move(measurement),
      .wrap_layout = std::move(wrap_layout),
  };
}

TextParagraphLayoutResult TextParagraphLayoutCache::layout(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale,
    float max_width) {
  const TextParagraphLayoutKey key{
      .text = std::string(text),
      .font = std::move(font),
      .font_size = font_size,
      .scale = normalized_scale(scale),
      .max_width = max_width,
  };
  lookup_count_ += 1;
  for (const auto& entry : entries_) {
    if (entry.key == key) {
      hit_count_ += 1;
      return TextParagraphLayoutResult{
          .layout = entry.layout,
          .cache_hit = true,
      };
    }
  }

  miss_count_ += 1;
  const TextParagraphLayout layout = layout_text_paragraph(
      key.text,
      key.font,
      key.font_size,
      DpiScale{.value = key.scale},
      key.max_width);
  entries_.push_back(Entry{.key = key, .layout = layout});
  return TextParagraphLayoutResult{
      .layout = layout,
      .cache_hit = false,
  };
}

std::size_t TextParagraphLayoutCache::entry_count() const {
  return entries_.size();
}

std::size_t TextParagraphLayoutCache::lookup_count() const {
  return lookup_count_;
}

std::size_t TextParagraphLayoutCache::hit_count() const {
  return hit_count_;
}

std::size_t TextParagraphLayoutCache::miss_count() const {
  return miss_count_;
}

void TextParagraphLayoutCache::clear() {
  entries_.clear();
}

} // namespace cgpui
