#pragma once

#include "cgpui/ui/text_shape.hpp"

#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct TextShapingRequest {
  std::string_view text;
  FontDescriptor font;
  std::vector<FontFaceDescriptor> font_fallback_faces;
  float font_size = 16.0F;
  DpiScale scale;
  TextShapingBackendSelection backend;
  TextShapingDirection direction = TextShapingDirection::left_to_right;
  TextShapingScript script = TextShapingScript::common;
  std::string language;
  float tab_width_in_spaces = 4.0F;
};

[[nodiscard]] TextShapeRun shape_text_with_selected_backend(
    TextShapingRequest request);

[[nodiscard]] TextShapeRun shape_text_with_harfbuzz(
    TextShapingRequest request);

[[nodiscard]] TextShapeRun shape_text_with_directwrite(
    TextShapingRequest request);

// Core Text on macOS (text_shaping_coretext_macos.cpp).
[[nodiscard]] TextShapeRun shape_text_with_core_text(
    TextShapingRequest request);

[[nodiscard]] TextShapeRun shape_text_with_deterministic_fallback(
    TextShapingRequest request);

} // namespace cgpui
