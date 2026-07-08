#include "cgpui/ui/text_shaping_backend.hpp"

namespace cgpui {

bool TextShapingBackendSelection::used_fallback() const {
  return requested != used;
}

TextShapingBackendCapabilities text_shaping_backend_capabilities() {
#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
  return TextShapingBackendCapabilities{.harfbuzz_available = true};
#else
  return TextShapingBackendCapabilities{};
#endif
}

TextShapingBackendSelection select_text_shaping_backend(
    TextShapingOptions options) {
  if (options.preferred_backend ==
      TextShapingBackend::deterministic_fallback) {
    return TextShapingBackendSelection{
        .requested = options.preferred_backend,
        .used = TextShapingBackend::deterministic_fallback,
        .fallback_reason = TextShapingFallbackReason::none,
    };
  }

  const TextShapingBackendCapabilities capabilities =
      text_shaping_backend_capabilities();
  if (capabilities.harfbuzz_available) {
    return TextShapingBackendSelection{
        .requested = options.preferred_backend,
        .used = TextShapingBackend::harfbuzz,
        .fallback_reason = TextShapingFallbackReason::none,
    };
  }

  return TextShapingBackendSelection{
      .requested = options.preferred_backend,
      .used = TextShapingBackend::deterministic_fallback,
      .fallback_reason = TextShapingFallbackReason::backend_unavailable,
  };
}

TextShapingDirection resolve_text_shaping_direction(
    TextShapingDirection direction) {
  return direction == TextShapingDirection::auto_direction
      ? TextShapingDirection::left_to_right
      : direction;
}

TextShapingScript resolve_text_shaping_script(TextShapingScript script) {
  return script == TextShapingScript::auto_script ? TextShapingScript::common
                                                  : script;
}

} // namespace cgpui
