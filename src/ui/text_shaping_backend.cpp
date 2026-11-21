#include "cgpui/ui/text_shaping_backend.hpp"

#include <atomic>

namespace cgpui {
namespace {

std::atomic<TextShapingBackend> default_backend{TextShapingBackend::native};

bool backend_available(
    TextShapingBackend backend,
    const TextShapingBackendCapabilities& capabilities) {
  switch (backend) {
    case TextShapingBackend::deterministic_fallback:
      return true;
    case TextShapingBackend::harfbuzz:
      return capabilities.harfbuzz_available;
    case TextShapingBackend::directwrite:
      return capabilities.directwrite_available;
    case TextShapingBackend::core_text:
      return capabilities.core_text_available;
    case TextShapingBackend::native:
      return false;
  }
  return false;
}

TextShapingBackend platform_native_backend(
    const TextShapingBackendCapabilities& capabilities) {
  if (capabilities.directwrite_available) {
    return TextShapingBackend::directwrite;
  }
  if (capabilities.core_text_available) {
    return TextShapingBackend::core_text;
  }
  if (capabilities.harfbuzz_available) {
    return TextShapingBackend::harfbuzz;
  }
  return TextShapingBackend::deterministic_fallback;
}

} // namespace

bool TextShapingBackendSelection::used_fallback() const {
  return requested != used;
}

TextShapingBackendCapabilities text_shaping_backend_capabilities() {
  TextShapingBackendCapabilities capabilities;
#if defined(CGPUI_HAS_HARFBUZZ_SHAPING_BACKEND)
  capabilities.harfbuzz_available = true;
#endif
#if defined(_WIN32)
  capabilities.directwrite_available = true;
#endif
#if defined(__APPLE__)
  capabilities.core_text_available = true;
#endif
  return capabilities;
}

void set_default_text_shaping_backend(TextShapingBackend backend) {
  default_backend.store(backend, std::memory_order_relaxed);
}

TextShapingBackend default_text_shaping_backend() {
  return default_backend.load(std::memory_order_relaxed);
}

TextShapingBackendSelection select_text_shaping_backend(
    TextShapingOptions options) {
  const TextShapingBackendCapabilities capabilities =
      text_shaping_backend_capabilities();
  TextShapingBackend requested = options.preferred_backend;
  if (requested == TextShapingBackend::native) {
    requested = default_text_shaping_backend();
  }
  TextShapingBackend candidate = requested;
  if (candidate == TextShapingBackend::native) {
    candidate = platform_native_backend(capabilities);
  }

  if (candidate == TextShapingBackend::deterministic_fallback &&
      requested != TextShapingBackend::deterministic_fallback) {
    return TextShapingBackendSelection{
        .requested = requested,
        .used = TextShapingBackend::deterministic_fallback,
        .fallback_reason = TextShapingFallbackReason::backend_unavailable,
        .capabilities = capabilities,
    };
  }
  if (backend_available(candidate, capabilities)) {
    return TextShapingBackendSelection{
        .requested = requested == TextShapingBackend::native ? candidate
                                                             : requested,
        .used = candidate,
        .fallback_reason = TextShapingFallbackReason::none,
        .capabilities = capabilities,
    };
  }

  return TextShapingBackendSelection{
      .requested = requested,
      .used = TextShapingBackend::deterministic_fallback,
      .fallback_reason = TextShapingFallbackReason::backend_unavailable,
      .capabilities = capabilities,
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
