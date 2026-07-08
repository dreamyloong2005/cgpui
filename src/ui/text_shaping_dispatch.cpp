#include "text_shaping_internal.hpp"

#include <utility>

namespace cgpui {

TextShapeRun shape_text_with_selected_backend(TextShapingRequest request) {
  if (request.backend.used == TextShapingBackend::harfbuzz) {
    return shape_text_with_harfbuzz(std::move(request));
  }
  return shape_text_with_deterministic_fallback(std::move(request));
}

} // namespace cgpui
