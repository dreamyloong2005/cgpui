#include "vulkan_report_internal.hpp"

#include <cstddef>

namespace cgpui {

std::vector<RendererCommandStreamItem> vulkan_build_renderer_command_stream(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws) {
  std::vector<RendererCommandStreamItem> commands;
  commands.reserve(
      rects.size() + rounded_rects.size() + text_draws.size() +
      selections.size() + carets.size() + image_draws.size());

  for (std::size_t index = 0; index < rects.size(); ++index) {
    const SolidRect& rect = rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::solid_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < rounded_rects.size(); ++index) {
    const RoundedRectDraw& rect = rounded_rects[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::rounded_rect,
        .command_index = index,
        .clip_rect = rect.clip_rect,
        .clip_stack = rect.clip_stack,
        .composition_stack = rect.composition_stack,
        .metadata = rect.metadata,
    });
  }

  for (std::size_t index = 0; index < text_draws.size(); ++index) {
    const TextDraw& text_draw = text_draws[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text,
        .command_index = index,
        .clip_rect = text_draw.clip_rect,
        .clip_stack = text_draw.clip_stack,
        .composition_stack = text_draw.composition_stack,
        .metadata = text_draw.metadata,
    });
  }

  for (std::size_t index = 0; index < selections.size(); ++index) {
    const TextSelectionDraw& selection = selections[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_selection,
        .command_index = index,
        .clip_rect = selection.clip_rect,
        .clip_stack = selection.clip_stack,
        .composition_stack = selection.composition_stack,
        .metadata = selection.metadata,
    });
  }

  for (std::size_t index = 0; index < carets.size(); ++index) {
    const TextCaretDraw& caret = carets[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::text_caret,
        .command_index = index,
        .clip_rect = caret.clip_rect,
        .clip_stack = caret.clip_stack,
        .composition_stack = caret.composition_stack,
        .metadata = caret.metadata,
    });
  }

  for (std::size_t index = 0; index < image_draws.size(); ++index) {
    const ImageDraw& image = image_draws[index];
    commands.push_back(RendererCommandStreamItem{
        .primitive_kind = RendererPrimitiveKind::image,
        .command_index = index,
        .clip_rect = image.clip_rect,
        .clip_stack = image.clip_stack,
        .composition_stack = image.composition_stack,
        .metadata = image.metadata,
    });
  }

  return commands;
}

} // namespace cgpui
