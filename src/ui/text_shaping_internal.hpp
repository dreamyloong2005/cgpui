#pragma once

#include "cgpui/ui/text_shape.hpp"

#include <string_view>

namespace cgpui {

struct TextShapingRequest {
  std::string_view text;
  FontDescriptor font;
  float font_size = 16.0F;
  DpiScale scale;
  TextShapingBackendSelection backend;
};

[[nodiscard]] TextShapeRun shape_text_with_selected_backend(
    TextShapingRequest request);

[[nodiscard]] TextShapeRun shape_text_with_deterministic_fallback(
    TextShapingRequest request);

} // namespace cgpui
