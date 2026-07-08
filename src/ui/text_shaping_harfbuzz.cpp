#include "text_shaping_internal.hpp"

#include <utility>

namespace cgpui {

TextShapeRun shape_text_with_harfbuzz(TextShapingRequest request) {
#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
#error "CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND requires the real HarfBuzz shaping implementation"
#endif
  request.backend.used = TextShapingBackend::deterministic_fallback;
  request.backend.fallback_reason = TextShapingFallbackReason::backend_unavailable;
  return shape_text_with_deterministic_fallback(std::move(request));
}

} // namespace cgpui
