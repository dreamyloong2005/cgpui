#include "text_shaping_internal.hpp"

#include "text_shape_cache_internal.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {
namespace {

bool request_uses_test_fonts(const TextShapingRequest& request) {
  return std::ranges::any_of(
      request.font_fallback_faces,
      [](const FontFaceDescriptor& face) {
        return face.source == FontSource::test;
      });
}

TextShapeRun shape_text_natively(TextShapingRequest request) {
  if (request.backend.used == TextShapingBackend::directwrite) {
    return shape_text_with_directwrite(std::move(request));
  }
  if (request.backend.used == TextShapingBackend::harfbuzz) {
    return shape_text_with_harfbuzz(std::move(request));
  }
  if (request.backend.used == TextShapingBackend::core_text) {
    return shape_text_with_core_text(std::move(request));
  }
  return shape_text_with_deterministic_fallback(std::move(request));
}

} // namespace

TextShapeRun shape_text_with_selected_backend(TextShapingRequest request) {
  if (request.backend.used == TextShapingBackend::deterministic_fallback) {
    return shape_text_with_deterministic_fallback(std::move(request));
  }
  if (request_uses_test_fonts(request)) {
    request.backend.used = TextShapingBackend::deterministic_fallback;
    request.backend.fallback_reason =
        TextShapingFallbackReason::unsupported_font_source;
    return shape_text_with_deterministic_fallback(std::move(request));
  }
  if (const TextShapeRun* cached = find_cached_text_shape(request);
      cached != nullptr) {
    return *cached;
  }
  const TextShapingRequest key = request;
  TextShapeRun run = shape_text_natively(std::move(request));
  if (run.used_backend != TextShapingBackend::deterministic_fallback) {
    store_cached_text_shape(key, run);
  }
  return run;
}

} // namespace cgpui
