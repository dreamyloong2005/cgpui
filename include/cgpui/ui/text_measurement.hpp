#pragma once

#include "cgpui/ui/text_shape.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct TextMeasurementKey {
  std::string text;
  FontDescriptor font;
  float font_size = 16.0F;
  float scale = 1.0F;

  friend bool operator==(
      const TextMeasurementKey&,
      const TextMeasurementKey&) = default;
};

struct TextGraphemeColumn {
  std::size_t column_start = 0;
  std::size_t column_end = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  std::size_t glyph_start = 0;
  std::size_t glyph_end = 0;
  float advance = 0.0F;
  float device_advance = 0.0F;
};

struct TextBidiRun {
  TextShapingDirection direction = TextShapingDirection::left_to_right;
  std::uint8_t embedding_level = 0;
  std::size_t column_start = 0;
  std::size_t column_end = 0;
  std::size_t byte_start = 0;
  std::size_t byte_end = 0;
  std::size_t glyph_start = 0;
  std::size_t glyph_end = 0;
  float advance = 0.0F;
  float device_advance = 0.0F;
};

struct TextMeasurement {
  TextShapeRun shape_run;
  Size logical_size;
  Size device_size;
  std::vector<TextGraphemeColumn> grapheme_columns;
  TextShapingDirection base_direction = TextShapingDirection::left_to_right;
  std::vector<TextBidiRun> bidi_runs;
};

struct TextMeasurementResult {
  TextMeasurement measurement;
  bool cache_hit = false;
};

[[nodiscard]] TextMeasurement measure_text(
    std::string_view text,
    FontDescriptor font = {},
    float font_size = 16.0F,
    DpiScale scale = {});

class TextMeasurementCache {
 public:
  [[nodiscard]] TextMeasurementResult measure(
      std::string_view text,
      FontDescriptor font = {},
      float font_size = 16.0F,
      DpiScale scale = {});

  [[nodiscard]] std::size_t entry_count() const;
  [[nodiscard]] std::size_t lookup_count() const;
  [[nodiscard]] std::size_t hit_count() const;
  [[nodiscard]] std::size_t miss_count() const;

  void clear();

 private:
  struct Entry {
    TextMeasurementKey key;
    TextMeasurement measurement;
  };

  std::vector<Entry> entries_;
  std::size_t lookup_count_ = 0;
  std::size_t hit_count_ = 0;
  std::size_t miss_count_ = 0;
};

} // namespace cgpui
