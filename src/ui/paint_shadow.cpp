#include "ui_internal.hpp"

namespace cgpui {

void PaintList::draw_box_shadow(
    Rect bounds,
    BoxShadow shadow,
    BorderRadii radius) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::box_shadow,
      .box_shadow =
          BoxShadowPaint{
              .bounds = bounds,
              .shadow = shadow,
              .radius = radius,
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::draw_backdrop_blur(
    Rect bounds,
    BorderRadii radius,
    float blur_radius,
    std::optional<Color> tint) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::backdrop_blur,
      .backdrop_blur =
          BackdropBlurPaint{
              .bounds = bounds,
              .radius = radius,
              .blur_radius = blur_radius,
              .tint = tint,
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

} // namespace cgpui
