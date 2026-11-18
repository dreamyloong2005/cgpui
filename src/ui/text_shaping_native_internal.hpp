#pragma once

#include "cgpui/ui/text_measurement.hpp"
#include "text_shaping_internal.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cgpui {

// One shaped glyph in logical pixels, logical order, y-down offset. The
// cluster is the UTF-8 byte range of the source text it belongs to.
struct NativeShapedGlyph {
  std::uint32_t glyph_id = 0;
  std::size_t cluster_start = 0;
  std::size_t cluster_end = 0;
  float advance = 0.0F;
  Point offset;
};

struct NativeShapedFace {
  FontFaceDescriptor face;
  TextFontMetrics metrics;
  std::uint32_t space_glyph_id = 0;
  float space_advance = 0.0F;
};

// Collects per-face glyph output from a platform shaper and builds the
// public TextShapeRun: fallback faces and runs, script runs, tab stops,
// control characters, missing-glyph and color-glyph diagnostics, metrics.
class NativeShapeBuilder {
 public:
  NativeShapeBuilder(TextShapingRequest& request, TextShapingBackend used);

  // Faces are deduplicated by file, index and synthesis. The first face
  // added is the primary face whose metrics describe the run.
  std::size_t add_face(NativeShapedFace face);
  void add_glyph(std::size_t face_slot, NativeShapedGlyph glyph);

  [[nodiscard]] TextShapeRun finish();

 private:
  TextShapingRequest& request_;
  TextShapingBackend used_;
  std::vector<NativeShapedFace> faces_;
  std::vector<std::pair<std::size_t, NativeShapedGlyph>> glyphs_;
};

// liga/calt default off for monospaced faces unless set explicitly.
[[nodiscard]] std::vector<FontFeature> native_font_features(
    const FontDescriptor& font,
    bool monospaced);

// Implemented by the platform font backend (DirectWrite, fontconfig).
[[nodiscard]] bool native_font_family_installed(std::string_view family);

[[nodiscard]] char32_t text_shaping_decode_utf8(
    std::string_view text,
    std::size_t byte_offset,
    std::size_t* byte_length);

[[nodiscard]] TextShapingScript text_shaping_script_for_codepoint(
    char32_t codepoint);

[[nodiscard]] TextLineMetrics native_text_line_metrics(
    const TextShapeRun& run);

} // namespace cgpui
