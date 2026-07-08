#pragma once

#include <string>

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

enum class TextShapingDirection {
  auto_direction,
  left_to_right,
  right_to_left,
  top_to_bottom,
  bottom_to_top,
};

enum class TextShapingScript {
  auto_script,
  common,
  inherited,
  latin,
  han,
  hiragana,
  katakana,
  hangul,
  arabic,
  hebrew,
  devanagari,
  emoji,
};

struct TextShapingOptions {
  TextShapingBackend preferred_backend = TextShapingBackend::harfbuzz;
  TextShapingDirection direction = TextShapingDirection::auto_direction;
  TextShapingScript script = TextShapingScript::auto_script;
  std::string language;
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
  TextShapingBackendCapabilities capabilities;

  [[nodiscard]] bool used_fallback() const;
};

[[nodiscard]] TextShapingBackendCapabilities text_shaping_backend_capabilities();

[[nodiscard]] TextShapingBackendSelection select_text_shaping_backend(
    TextShapingOptions options = {});

[[nodiscard]] TextShapingDirection resolve_text_shaping_direction(
    TextShapingDirection direction);

[[nodiscard]] TextShapingScript resolve_text_shaping_script(
    TextShapingScript script);

} // namespace cgpui
