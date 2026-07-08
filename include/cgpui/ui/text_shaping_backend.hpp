#pragma once

namespace cgpui {

enum class TextShapingBackend {
  deterministic_fallback,
  harfbuzz,
};

enum class TextShapingFallbackReason {
  none,
  backend_unavailable,
  shaping_failed,
};

struct TextShapingOptions {
  TextShapingBackend preferred_backend = TextShapingBackend::harfbuzz;
};

struct TextShapingBackendCapabilities {
  // True only when a real HarfBuzz shaping backend is compiled in.
  bool harfbuzz_available = false;
};

struct TextShapingBackendSelection {
  TextShapingBackend requested = TextShapingBackend::harfbuzz;
  TextShapingBackend used = TextShapingBackend::deterministic_fallback;
  TextShapingFallbackReason fallback_reason =
      TextShapingFallbackReason::backend_unavailable;

  [[nodiscard]] bool used_fallback() const;
};

[[nodiscard]] TextShapingBackendCapabilities text_shaping_backend_capabilities();

[[nodiscard]] TextShapingBackendSelection select_text_shaping_backend(
    TextShapingOptions options = {});

} // namespace cgpui
