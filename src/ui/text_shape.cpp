#include "text_shaping_internal.hpp"

#include <utility>

namespace cgpui {

std::size_t TextShapeRun::glyph_count() const {
  return glyphs.size();
}

bool TextShapeRun::used_fallback() const {
  return requested_backend != used_backend;
}

TextShapingBackendSelection TextShapeRun::backend_selection() const {
  return TextShapingBackendSelection{
      .requested = requested_backend,
      .used = used_backend,
      .fallback_reason = fallback_reason,
      .capabilities = backend_capabilities,
  };
}

bool is_utf8_continuation_byte(char value) {
  return (static_cast<unsigned char>(value) & 0xC0U) == 0x80U;
}

TextShapeRun shape_text(
    std::string_view text,
    FontDescriptor font,
    float font_size,
    DpiScale scale,
    TextShapingOptions options) {
  const TextShapingBackendSelection backend =
      select_text_shaping_backend(options);
  return shape_text_with_selected_backend(TextShapingRequest{
      .text = text,
      .font = std::move(font),
      .font_size = font_size,
      .scale = scale,
      .backend = backend,
      .direction = resolve_text_shaping_direction(options.direction),
      .script = resolve_text_shaping_script(options.script),
      .language = std::move(options.language),
  });
}

} // namespace cgpui
