#include "cgpui/ui/text_measurement.hpp"

#include "text_measurement_internal.hpp"

#include <string>
#include <utility>

namespace cgpui {

TextMeasurement measure_text(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale) {
  TextShapeRun shape_run = shape_text(text, std::move(font), font_size, scale);
  const Size logical_size{
      .width = shape_run.total_advance,
      .height = shape_run.line_height,
  };
  const Size device_size{
      .width = shape_run.device_total_advance,
      .height = shape_run.device_line_height,
  };
  std::vector<TextGraphemeColumn> grapheme_columns =
      build_text_grapheme_columns(shape_run);
  std::vector<TextBidiRun> bidi_runs =
      build_text_bidi_runs(shape_run, grapheme_columns);
  const TextShapingDirection base_direction = shape_run.direction;
  return TextMeasurement{
      .shape_run = std::move(shape_run),
      .logical_size = logical_size,
      .device_size = device_size,
      .grapheme_columns = std::move(grapheme_columns),
      .base_direction = base_direction,
      .bidi_runs = std::move(bidi_runs),
  };
}

TextMeasurementResult TextMeasurementCache::measure(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale) {
  const TextMeasurementKey key{
      .text = std::string(text),
      .font = std::move(font),
      .font_size = font_size,
      .scale = normalized_scale(scale),
  };
  lookup_count_ += 1;
  for (const auto& entry : entries_) {
    if (entry.key == key) {
      hit_count_ += 1;
      return TextMeasurementResult{
          .measurement = entry.measurement,
          .cache_hit = true,
      };
    }
  }

  miss_count_ += 1;
  const TextMeasurement measurement = measure_text(
      key.text,
      key.font,
      key.font_size,
      DpiScale{.value = key.scale});
  entries_.push_back(Entry{.key = key, .measurement = measurement});
  return TextMeasurementResult{
      .measurement = measurement,
      .cache_hit = false,
  };
}

std::size_t TextMeasurementCache::entry_count() const {
  return entries_.size();
}

std::size_t TextMeasurementCache::lookup_count() const {
  return lookup_count_;
}

std::size_t TextMeasurementCache::hit_count() const {
  return hit_count_;
}

std::size_t TextMeasurementCache::miss_count() const {
  return miss_count_;
}

void TextMeasurementCache::clear() {
  entries_.clear();
}

} // namespace cgpui
