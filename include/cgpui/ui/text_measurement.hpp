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

struct TextLineMetrics {
  float ascent = 0.0F;
  float descent = 0.0F;
  float leading = 0.0F;
  float line_height = 0.0F;
  float baseline = 0.0F;
  float device_ascent = 0.0F;
  float device_descent = 0.0F;
  float device_leading = 0.0F;
  float device_line_height = 0.0F;
  float device_baseline = 0.0F;
};

struct TextMeasurement {
  TextShapeRun shape_run;
  Size logical_size;
  Size device_size;
  std::vector<TextGraphemeColumn> grapheme_columns;
  TextShapingDirection base_direction = TextShapingDirection::left_to_right;
  std::vector<TextBidiRun> bidi_runs;
  TextLineMetrics line_metrics;
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

[[nodiscard]] TextLineMetrics text_line_metrics_for_shape_run(
    const TextShapeRun& run);

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
