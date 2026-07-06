#include "ui_internal.hpp"

#include "paint_clip.hpp"

namespace cgpui {

void PaintList::clear() {
  commands_.clear();
  clip_stack_.clear();
  metadata_stack_.clear();
  scale_ = {};
}

void PaintList::set_scale(DpiScale scale) {
  scale_ = scale;
}

DpiScale PaintList::scale() const {
  return scale_;
}

void PaintList::set_text_measurement_cache(TextMeasurementCache* cache) {
  text_measurement_cache_ = cache;
}

void PaintList::push_clip(Rect rect) {
  clip_stack_.push_back(effective_nested_clip_rect(clip_stack_, rect));
}

void PaintList::pop_clip() {
  if (!clip_stack_.empty()) {
    clip_stack_.pop_back();
  }
}

void PaintList::push_metadata(PaintMetadata metadata) {
  if (!metadata_stack_.empty()) {
    metadata = compose_paint_metadata(metadata_stack_.back(), metadata);
  }
  metadata_stack_.push_back(metadata);
}

void PaintList::pop_metadata() {
  if (!metadata_stack_.empty()) {
    metadata_stack_.pop_back();
  }
}

void PaintList::fill_rect(Rect rect, Color color) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::solid_rect,
      .solid_rect = SolidRect{.rect = rect, .color = color},
      .rounded_rect = RoundedRect{},
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

void PaintList::fill_rounded_rect(Rect rect, Color color, BorderRadii radius) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::rounded_rect,
      .solid_rect = SolidRect{.rect = rect, .color = color},
      .rounded_rect =
          RoundedRect{
              .rect = rect,
              .color = color,
              .radius = radius,
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

std::span<const PaintCommand> PaintList::commands() const {
  return commands_;
}


} // namespace cgpui
