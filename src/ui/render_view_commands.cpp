#include "ui_internal.hpp"

namespace cgpui {

void submit_paint_command_to_frame(
    RenderFrame& frame,
    const PaintCommand& command,
    FrameStatistics* statistics) {
  if (statistics != nullptr) {
    record_clip_stack_statistics(*statistics, command);
    record_composition_stack_statistics(*statistics, command);
  }
  if (command.kind == PaintCommandKind::box_shadow) {
    return;
  }
  if (command.kind == PaintCommandKind::image) {
    const ImagePaint& image = command.image;
    frame.draw_image(ImageDraw{
        .bounds = image.bounds,
        .asset = image.asset,
        .source_rect = image.source_rect,
        .tint = image.tint,
        .sampling = image.sampling,
        .clip_rect = command.clip_rect,
        .clip_stack = command.clip_stack,
        .composition_stack = command.composition_stack,
        .metadata = command.metadata,
    });
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->image_command_count += 1;
    }
    return;
  }
  if (command.kind == PaintCommandKind::text_selection) {
    const TextSelectionPaint& selection = command.text_selection;
    frame.draw_text_selection(TextSelectionDraw{
        .rect = selection.rect,
        .color = selection.color,
        .range = selection.range,
        .font_size = selection.font_size,
        .clip_rect = command.clip_rect,
        .clip_stack = command.clip_stack,
        .composition_stack = command.composition_stack,
        .metadata = command.metadata,
    });
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->text_selection_command_count += 1;
    }
    return;
  }
  if (command.kind == PaintCommandKind::text_caret) {
    const TextCaretPaint& caret = command.text_caret;
    frame.draw_text_caret(TextCaretDraw{
        .rect = caret.rect,
        .color = caret.color,
        .byte_offset = caret.byte_offset,
        .font_size = caret.font_size,
        .clip_rect = command.clip_rect,
        .clip_stack = command.clip_stack,
        .composition_stack = command.composition_stack,
        .metadata = command.metadata,
    });
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->text_caret_command_count += 1;
    }
    return;
  }
  if (command.kind == PaintCommandKind::text) {
    const TextPaint& text = command.text;
    frame.draw_text(TextDraw{
        .bounds = text.bounds,
        .color = text.color,
        .font = text.font,
        .content = text.content,
        .byte_length = text.byte_length,
        .font_size = text.font_size,
        .scale = text.scale,
        .device_font_size = text.device_font_size,
        .glyphs = text.glyphs,
        .lines = text.lines,
        .rich_text_runs = text.rich_text_runs,
        .rich_text_inline_images = text.rich_text_inline_images,
        .clip_rect = command.clip_rect,
        .clip_stack = command.clip_stack,
        .composition_stack = command.composition_stack,
        .metadata = command.metadata,
    });
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->text_command_count += 1;
    }
    return;
  }
  if (command.kind == PaintCommandKind::rounded_rect) {
    const RoundedRect& rect = command.rounded_rect;
    frame.draw_rounded_rect(RoundedRectDraw{
        .rect = rect.rect,
        .color = rect.color,
        .radius = rect.radius,
        .fill_enabled = rect.fill_enabled,
        .border_color = rect.border_color,
        .border_width = rect.border_width,
        .clip_rect = command.clip_rect,
        .clip_stack = command.clip_stack,
        .composition_stack = command.composition_stack,
        .metadata = command.metadata,
    });
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->rounded_rect_command_count += 1;
    }
    return;
  }
  SolidRect rect = command.solid_rect;
  rect.clip_rect = command.clip_rect;
  rect.clip_stack = command.clip_stack;
  rect.composition_stack = command.composition_stack;
  rect.metadata = command.metadata;
  frame.draw_rect(rect);
  if (statistics != nullptr) {
    statistics->submitted_command_count += 1;
    statistics->solid_rect_command_count += 1;
  }
}

} // namespace cgpui
