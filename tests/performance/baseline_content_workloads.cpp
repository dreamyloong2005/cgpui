#include "baseline_measure.hpp"
#include "baseline_workloads.hpp"

#include "cgpui/prelude.hpp"

#include <array>
#include <cmath>
#include <string>

namespace performance_baseline {
namespace {
std::vector<std::uint8_t> png_bytes() {
  constexpr std::string_view encoded =
      "iVBORw0KGgoAAAANSUhEUgAAAAIAAAABCAYAAAD0In+KAAAAAXNSR0IArs4c6QAA"
      "AARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAOSURBVBhXY/jP"
      "wPAfBAEQ+AP9OGZFWQAAAABJRU5ErkJggg==";
  const auto value = [](char character) {
    if (character >= 'A' && character <= 'Z') return character - 'A';
    if (character >= 'a' && character <= 'z') return character - 'a' + 26;
    if (character >= '0' && character <= '9') return character - '0' + 52;
    if (character == '+') return 62;
    if (character == '/') return 63;
    return -1;
  };
  std::vector<std::uint8_t> bytes;
  std::uint32_t accumulator = 0;
  int bits = -8;
  for (const char character : encoded) {
    if (character == '=') break;
    const int next = value(character);
    if (next < 0) continue;
    accumulator = (accumulator << 6) | static_cast<std::uint32_t>(next);
    bits += 6;
    if (bits >= 0) {
      bytes.push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xff));
      bits -= 8;
    }
  }
  return bytes;
}
} // namespace

void append_content_samples(std::vector<Sample>& samples, std::size_t iterations) {
  constexpr std::string_view paragraph =
      "CGPUI public text layout baseline: Latin, CJK, emoji, wrapping, and bidi metadata.";
  std::size_t layouts = 0;
  samples.push_back(measure("text_layout", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto measured = cgpui::measure_text(paragraph, {}, 16.0F, cgpui::DpiScale{1.0F});
      if (!measured.shape_run.glyphs.empty() &&
          measured.grapheme_columns.size() == paragraph.size() &&
          measured.grapheme_columns.front().byte_start == 0 &&
          measured.grapheme_columns.back().byte_end == paragraph.size() &&
          std::isfinite(measured.logical_size.width) && measured.logical_size.width > 0 &&
          std::isfinite(measured.logical_size.height) && measured.logical_size.height > 0) ++layouts;
    }
  }, [&] { return WorkloadObservation{layouts, true}; }));

  std::vector<cgpui::UniformListItemIdentity> items;
  items.reserve(10000);
  for (std::size_t index = 0; index < 10000; ++index) {
    items.push_back({.index = index, .key = {.value = std::to_string(index)},
        .element_id = cgpui::ElementId{index + 1},
        .content_bounds = {.origin = {0.0F, index * 20.0F}, .size = {400.0F, 20.0F}}});
  }
  // Independent worked examples for a 600px viewport and 20px rows, including
  // a partially visible row. An empty/fixed range is not successful scrolling.
  constexpr std::array<float, 4> offsets{0, 137, 400, 190000};
  constexpr std::array<std::size_t, 4> starts{0, 6, 20, 9500};
  constexpr std::array<std::size_t, 4> ends{30, 37, 50, 9530};
  std::size_t scrolled = 0;
  samples.push_back(measure("list_scrolling", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto example = index % offsets.size();
      const auto visible = cgpui::calculate_uniform_list_visible_range(
          items, {0, offsets[example]}, {400, 600});
      if (visible.start_index == starts[example] && visible.end_index == ends[example]) ++scrolled;
    }
  }, [&] { return WorkloadObservation{scrolled, true}; }));

  const auto encoded = png_bytes();
  const std::vector<std::uint8_t> expected_pixels{255, 0, 0, 255, 0, 255, 0, 255};
  std::size_t decoded_images = 0;
  samples.push_back(measure("image_loading", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto decoded = cgpui::decode_image(encoded);
      if (decoded.ready() && decoded.source_format == cgpui::EncodedImageFormat::png &&
          decoded.bitmap.width == 2 && decoded.bitmap.height == 1 && decoded.bitmap.stride == 8 &&
          decoded.bitmap.format == cgpui::ImageFormat::rgba8_unorm &&
          decoded.bitmap.pixels == expected_pixels) ++decoded_images;
    }
  }, [&] { return WorkloadObservation{decoded_images, true}; }));
}

} // namespace performance_baseline
