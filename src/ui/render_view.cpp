#include "ui_internal.hpp"

namespace cgpui {

void StyledElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const Style& base_style = style();
  paint_list.push_metadata(paint_metadata_for_style(base_style));
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && base_style.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(base_style.clip_rect.has_value()
                             ? *base_style.clip_rect
                             : *bounds);
  }
  paint_styled_box_base(paint_list, bounds, base_style);
  if (child_ != nullptr) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void ButtonElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  const Style& base_style = style_state_.base;
  paint_list.push_metadata(paint_metadata_for_style(base_style));
  const bool uses_hidden_overflow_clip =
      bounds.has_value() && base_style.overflow == Overflow::hidden;
  if (uses_hidden_overflow_clip) {
    paint_list.push_clip(base_style.clip_rect.has_value()
                             ? *base_style.clip_rect
                             : *bounds);
  }
  paint_styled_box_base(paint_list, bounds, base_style);
  if (child_) {
    child_->paint(paint_list);
  }
  if (uses_hidden_overflow_clip) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void ScrollableListElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  paint_list.push_metadata(paint_metadata_for_style(style_));
  if (bounds.has_value()) {
    paint_list.push_clip(*bounds);
  }
  for (const auto& child : content_.children()) {
    if (child) {
      child->paint(paint_list);
    }
  }
  if (bounds.has_value()) {
    paint_list.pop_clip();
  }
  paint_list.pop_metadata();
}

void LabelElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || text().empty()) {
    return;
  }
  const Color text_color = style().foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(style()));
  paint_list.fill_text(*bounds, text_color, text(), style().font, font_size());
  paint_list.pop_metadata();
}

void TextElement::paint(PaintList& paint_list) const {
  const std::optional<Rect> bounds = layout_bounds();
  if (!bounds.has_value() || model_ == nullptr) {
    return;
  }
  const Style& text_style = style();
  const Color text_color = text_style.foreground_color.value_or(
      Color{.r = 0.82F, .g = 0.86F, .b = 0.92F, .a = 1.0F});
  paint_list.push_metadata(paint_metadata_for_style(text_style));
  const float font_size_value = text_style.font_size;
  const float glyph_width_value = glyph_width();
  const TextSelectionRange selection = model_->selection();
  if (!selection.collapsed) {
    paint_list.fill_text_selection(
        Rect{
            .origin =
                {
                    .x = bounds->origin.x +
                         (static_cast<float>(selection.start) *
                          glyph_width_value),
                    .y = bounds->origin.y,
                },
            .size =
                {
                    .width = static_cast<float>(selection.end - selection.start) *
                             glyph_width_value,
                    .height = font_size_value,
                },
        },
        Color{.r = 0.22F, .g = 0.42F, .b = 0.80F, .a = 0.38F},
        selection,
        font_size_value);
  }
  if (!text().empty()) {
    paint_list.fill_text(
        *bounds,
        text_color,
        text(),
        text_style.font,
        font_size_value);
  }
  paint_list.fill_text_caret(
      Rect{
          .origin =
              {
                  .x = bounds->origin.x +
                       (static_cast<float>(model_->cursor()) *
                        glyph_width_value),
                  .y = bounds->origin.y,
              },
          .size = {.width = 1.0F, .height = font_size_value},
      },
      text_color,
      model_->cursor(),
      font_size_value);
  paint_list.pop_metadata();
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    FrameStatistics* statistics) {
  return render_view(renderer, view, viewport_size, DpiScale{}, statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    FrameStatistics* statistics) {
  return render_view(
      renderer,
      view,
      viewport_size,
      scale,
      nullptr,
      statistics);
}

Result<void> render_view(
    Renderer& renderer,
    View& view,
    Size viewport_size,
    DpiScale scale,
    TextMeasurementCache* text_measurement_cache,
    FrameStatistics* statistics) {
  if (statistics != nullptr) {
    statistics->begin_frame_count += 1;
  }
  auto frame = renderer.begin_frame();
  if (!frame) {
    return std::unexpected(frame.error());
  }
  if (!*frame) {
    return std::unexpected(Error{
        .code = ErrorCode::frame_acquisition_failed,
        .message = "Renderer returned an empty frame",
    });
  }

  (*frame)->clear(Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  if (statistics != nullptr) {
    statistics->clear_count += 1;
  }

  PaintList paint_list;
  paint_list.set_scale(scale);
  paint_list.set_text_measurement_cache(text_measurement_cache);
  view.paint(paint_list, viewport_size);
  if (statistics != nullptr) {
    statistics->paint_pass_count += 1;
    statistics->paint_command_count = paint_list.commands().size();
  }
  for (const auto& command : paint_list.commands()) {
    if (statistics != nullptr) {
      record_clip_stack_statistics(*statistics, command);
      record_composition_stack_statistics(*statistics, command);
    }
    if (command.kind == PaintCommandKind::image) {
      const ImagePaint& image = command.image;
      (*frame)->draw_image(ImageDraw{
          .bounds = image.bounds,
          .asset = image.asset,
          .source_rect = image.source_rect,
          .clip_rect = command.clip_rect,
          .clip_stack = command.clip_stack,
          .composition_stack = command.composition_stack,
          .metadata = command.metadata,
      });
      if (statistics != nullptr) {
        statistics->submitted_command_count += 1;
        statistics->image_command_count += 1;
      }
      continue;
    }
    if (command.kind == PaintCommandKind::text_selection) {
      const TextSelectionPaint& selection = command.text_selection;
      (*frame)->draw_text_selection(TextSelectionDraw{
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
      continue;
    }
    if (command.kind == PaintCommandKind::text_caret) {
      const TextCaretPaint& caret = command.text_caret;
      (*frame)->draw_text_caret(TextCaretDraw{
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
      continue;
    }
    if (command.kind == PaintCommandKind::text) {
      const TextPaint& text = command.text;
      (*frame)->draw_text(TextDraw{
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
          .clip_rect = command.clip_rect,
          .clip_stack = command.clip_stack,
          .composition_stack = command.composition_stack,
          .metadata = command.metadata,
      });
      if (statistics != nullptr) {
        statistics->submitted_command_count += 1;
        statistics->text_command_count += 1;
      }
      continue;
    }
    if (command.kind == PaintCommandKind::rounded_rect) {
      const RoundedRect& rect = command.rounded_rect;
      (*frame)->draw_rounded_rect(RoundedRectDraw{
          .rect = rect.rect,
          .color = rect.color,
          .radius = rect.radius,
          .clip_rect = command.clip_rect,
          .clip_stack = command.clip_stack,
          .composition_stack = command.composition_stack,
          .metadata = command.metadata,
      });
      if (statistics != nullptr) {
        statistics->submitted_command_count += 1;
        statistics->rounded_rect_command_count += 1;
      }
      continue;
    }
    SolidRect rect = command.solid_rect;
    rect.clip_rect = command.clip_rect;
    rect.clip_stack = command.clip_stack;
    rect.composition_stack = command.composition_stack;
    rect.metadata = command.metadata;
    (*frame)->draw_rect(rect);
    if (statistics != nullptr) {
      statistics->submitted_command_count += 1;
      statistics->solid_rect_command_count += 1;
    }
  }

  auto result = (*frame)->present();
  if (result && statistics != nullptr) {
    statistics->present_count += 1;
  }
  return result;
}


} // namespace cgpui
