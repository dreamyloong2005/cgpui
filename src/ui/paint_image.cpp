#include "ui_internal.hpp"

namespace cgpui {

void PaintList::draw_image(
    Rect bounds,
    ImageAssetDescriptor asset,
    std::optional<Rect> source_rect,
    std::optional<Color> tint) {
  commands_.push_back(PaintCommand{
      .kind = PaintCommandKind::image,
      .image =
          ImagePaint{
              .bounds = bounds,
              .asset = asset,
              .source_rect = source_rect,
              .tint = tint,
          },
      .clip_rect = current_clip_rect_for(clip_stack_),
      .clip_stack = clip_stack_record_for(clip_stack_),
      .composition_stack = composition_stack_record_for(metadata_stack_),
      .metadata = metadata_stack_.empty() ? PaintMetadata{}
                                          : metadata_stack_.back()});
}

} // namespace cgpui
