#pragma once

#include <string>

namespace cgpui {

// `native` requests the platform shaper: DirectWrite on Windows, HarfBuzz
// (with FreeType and fontconfig) on Linux. Platforms without a compiled
// backend use `deterministic_fallback` with `backend_unavailable`.
enum class TextShapingBackend {
  deterministic_fallback,
  harfbuzz,
  directwrite,
  native,
};

enum class TextShapingFallbackReason {
  none,
  backend_unavailable,
  shaping_failed,
  // Test font faces have no native font file; they shape deterministically.
  unsupported_font_source,
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
  TextShapingBackend preferred_backend = TextShapingBackend::native;
  TextShapingDirection direction = TextShapingDirection::auto_direction;
  TextShapingScript script = TextShapingScript::auto_script;
  std::string language;
  // Tab stops are this many space advances of the primary face apart,
  // measured from the start of the shaped run.
  float tab_width_in_spaces = 4.0F;
};

struct TextShapingBackendCapabilities {
  // True only when a real HarfBuzz shaping backend is compiled in.
  bool harfbuzz_available = false;
  bool directwrite_available = false;
};

struct TextShapingBackendSelection {
  TextShapingBackend requested = TextShapingBackend::native;
  TextShapingBackend used = TextShapingBackend::deterministic_fallback;
  TextShapingFallbackReason fallback_reason =
      TextShapingFallbackReason::backend_unavailable;
  TextShapingBackendCapabilities capabilities;

  [[nodiscard]] bool used_fallback() const;
};

[[nodiscard]] TextShapingBackendCapabilities text_shaping_backend_capabilities();

// Process-wide backend used for `TextShapingBackend::native` requests.
// Defaults to `native`; tests that assert fixed advances set
// `deterministic_fallback`.
void set_default_text_shaping_backend(TextShapingBackend backend);
[[nodiscard]] TextShapingBackend default_text_shaping_backend();

[[nodiscard]] TextShapingBackendSelection select_text_shaping_backend(
    TextShapingOptions options = {});

[[nodiscard]] TextShapingDirection resolve_text_shaping_direction(
    TextShapingDirection direction);

[[nodiscard]] TextShapingScript resolve_text_shaping_script(
    TextShapingScript script);

} // namespace cgpui
