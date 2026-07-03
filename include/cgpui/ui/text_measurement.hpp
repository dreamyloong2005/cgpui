#pragma once

#include "cgpui/ui/text_shape.hpp"

#include <cstddef>
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

struct TextMeasurement {
  TextShapeRun shape_run;
  Size logical_size;
  Size device_size;
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
