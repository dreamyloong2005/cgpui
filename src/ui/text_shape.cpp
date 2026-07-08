#include "text_shaping_internal.hpp"

#include <utility>
#include <vector>

namespace cgpui {
namespace {

std::vector<FontFaceDescriptor> copy_font_fallback_faces(
    const FontFallbackChain& fallback_chain) {
  std::vector<FontFaceDescriptor> faces;
  faces.reserve(fallback_chain.size());
  for (const FontFaceDescriptor* face : fallback_chain.faces()) {
    if (face != nullptr) {
      faces.push_back(*face);
    }
  }
  return faces;
}

} // namespace

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

TextShapeRun shape_text(
    std::string_view text,
    const FontFallbackChain& fallback_chain,
    float font_size,
    DpiScale scale,
    TextShapingOptions options) {
  std::vector<FontFaceDescriptor> font_fallback_faces =
      copy_font_fallback_faces(fallback_chain);
  FontDescriptor primary_font;
  if (!font_fallback_faces.empty()) {
    primary_font = font_fallback_faces.front().font;
  }

  const TextShapingBackendSelection backend =
      select_text_shaping_backend(options);
  return shape_text_with_selected_backend(TextShapingRequest{
      .text = text,
      .font = std::move(primary_font),
      .font_fallback_faces = std::move(font_fallback_faces),
      .font_size = font_size,
      .scale = scale,
      .backend = backend,
      .direction = resolve_text_shaping_direction(options.direction),
      .script = resolve_text_shaping_script(options.script),
      .language = std::move(options.language),
  });
}

} // namespace cgpui
