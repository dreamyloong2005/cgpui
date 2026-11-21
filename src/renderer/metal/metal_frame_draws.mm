#include "metal_effects_internal.hpp"

namespace cgpui {

// Encodes one recorded draw. A backdrop ends `encoder` and replaces it
// with the encoder that continues the scene; on failure the current
// encoder is ended. Returns the glyph atlas bytes uploaded.
Result<std::size_t> metal_encode_frame_draw(
    id<MTLCommandBuffer> command_buffer,
    __strong id<MTLRenderCommandEncoder>& encoder,
    MetalRendererState& state,
    const MetalFrameCommands& commands,
    MetalFrameDrawOrderEntry entry) {
  Result<void> encoded;
  std::size_t uploaded = 0;
  switch (entry.primitive_kind) {
    case RendererPrimitiveKind::solid_rect:
      encoded = metal_encode_solid_rect(
          encoder, state, commands.solid_rects[entry.command_index]);
      break;
    case RendererPrimitiveKind::rounded_rect:
      encoded = metal_encode_rounded_rect(
          encoder, state, commands.rounded_rects[entry.command_index]);
      break;
    case RendererPrimitiveKind::text: {
      auto text_upload = metal_encode_text(
          encoder, state, commands.text_draws[entry.command_index]);
      if (!text_upload) encoded = std::unexpected(text_upload.error());
      else uploaded = *text_upload;
      break;
    }
    case RendererPrimitiveKind::text_selection: {
      const TextSelectionDraw& selection =
          commands.selections[entry.command_index];
      encoded = metal_encode_solid_rect(
          encoder,
          state,
          SolidRect{
              .rect = selection.rect,
              .color = selection.color,
              .clip_rect = selection.clip_rect,
              .clip_stack = selection.clip_stack,
              .composition_stack = selection.composition_stack,
              .metadata = selection.metadata});
      break;
    }
    case RendererPrimitiveKind::text_caret: {
      const TextCaretDraw& caret = commands.carets[entry.command_index];
      encoded = metal_encode_solid_rect(
          encoder,
          state,
          SolidRect{
              .rect = caret.rect,
              .color = caret.color,
              .clip_rect = caret.clip_rect,
              .clip_stack = caret.clip_stack,
              .composition_stack = caret.composition_stack,
              .metadata = caret.metadata});
      break;
    }
    case RendererPrimitiveKind::image:
      encoded = metal_encode_image(
          encoder, state, commands.image_draws[entry.command_index]);
      break;
    case RendererPrimitiveKind::box_shadow:
      encoded = metal_encode_box_shadow(
          encoder, state, commands.box_shadows[entry.command_index]);
      break;
    case RendererPrimitiveKind::backdrop_blur: {
      auto next = metal_encode_backdrop_blur(
          command_buffer, encoder, state,
          commands.backdrops[entry.command_index]);
      if (!next) return std::unexpected(next.error());
      encoder = *next;
      break;
    }
  }
  if (!encoded) {
    [encoder endEncoding];
    return std::unexpected(encoded.error());
  }
  return uploaded;
}

}  // namespace cgpui
