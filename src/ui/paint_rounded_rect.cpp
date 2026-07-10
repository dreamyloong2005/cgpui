#include "ui_internal.hpp"

namespace cgpui {

void PaintList::append_rounded_rect(
    Rect rect,
    Color fill_color,
    BorderRadii radius,
    bool fill_enabled,
    std::optional<Color> border_color,
    float border_width) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::rounded_rect,
      .solid_rect = SolidRect{.rect = rect, .color = fill_color},
      .rounded_rect =
          RoundedRect{
              .rect = rect,
              .color = fill_color,
              .radius = radius,
              .fill_enabled = fill_enabled,
              .border_color = border_color,
              .border_width = border_width,
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_rounded_rect(
    Rect rect,
    Color color,
    BorderRadii radius) {
  append_rounded_rect(rect, color, radius, true, std::nullopt, 0.0F);
}

void PaintList::fill_stroked_rounded_rect(
    Rect rect,
    Color fill_color,
    BorderRadii radius,
    Color border_color,
    float border_width) {
  if (border_width <= 0.0F) {
    fill_rounded_rect(rect, fill_color, radius);
    return;
  }
  append_rounded_rect(
      rect, fill_color, radius, true, border_color, border_width);
}

void PaintList::stroke_rounded_rect(
    Rect rect,
    Color border_color,
    BorderRadii radius,
    float border_width) {
  if (border_width <= 0.0F) {
    return;
  }
  append_rounded_rect(
      rect,
      Color{.a = 0.0F},
      radius,
      false,
      border_color,
      border_width);
}

} // namespace cgpui
