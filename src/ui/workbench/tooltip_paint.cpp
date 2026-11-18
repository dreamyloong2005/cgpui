#include "cgpui/ui/tooltip.hpp"

#include "cgpui/ui/anchored.hpp"
#include "cgpui/ui/paint.hpp"
#include "cgpui/ui/text_measurement.hpp"
#include "workbench_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

void Tooltip::layout(Size viewport, DpiScale scale, const WorkbenchTheme& theme) {
  live_->reduced_motion = theme.reduced_motion;
  const Target* target = active_.has_value() ? find(*active_) : nullptr;
  if (target == nullptr || !(live_->visible || live_->pending)) {
    bounds_ = {};
    flipped_ = false;
    return;
  }
  // Re-measure only when the text, font size or scale changed.
  if (measured_target_ != target->id || measured_text_ != target->text ||
      font_size_ != theme.font_size || scale_.value != scale.value) {
    text_width_ = measure_text(target->text, theme.font, theme.font_size, scale)
                      .logical_size.width;
    measured_target_ = target->id;
    measured_text_ = target->text;
    font_size_ = theme.font_size;
  }
  scale_ = scale;
  // Whole device pixels, so the snapped bubble stays inside the margins.
  const float factor = workbench_detail::scale_of(scale);
  const auto device_ceil = [factor](float value) {
    return std::ceil(value * factor) / factor;
  };
  const Size size{
      device_ceil(std::min(options_.max_width, text_width_ + options_.padding * 2.0F)),
      device_ceil(theme.font_size * 1.4F + options_.padding * 2.0F)};
  const Rect anchor = options_.position == TooltipPosition::anchor
      ? target->bounds
      : Rect{{anchor_pointer_.x, anchor_pointer_.y - 2.0F},
             {1.0F, options_.pointer_offset}};
  const AnchoredBounds placed = anchored_bounds_beside(
      anchor, AnchoredSide::below, size, viewport,
      options_.position == TooltipPosition::anchor ? options_.gap : 0.0F);
  bounds_ = Rect{{workbench_detail::snap(placed.bounds.origin.x, scale),
                  workbench_detail::snap(placed.bounds.origin.y, scale)},
                 placed.bounds.size};
  flipped_ = placed.flipped_y;
}

Rect Tooltip::bounds() const {
  return bounds_;
}

bool Tooltip::flipped() const {
  return flipped_;
}

// A non-interactive node: tooltips never take hits or focus.
void Tooltip::append_elements(
    WidgetTreeBuilder& builder,
    WidgetTreeBuilder::NodeHandle parent) {
  const Target* target = active_.has_value() ? find(*active_) : nullptr;
  if (!live_->visible || target == nullptr) return;
  builder.add(parent, WidgetNode{.role = AccessibilityRole::tooltip,
                                 .name = target->text,
                                 .bounds = bounds_,
                                 .z_order = 950});
}

void Tooltip::paint(PaintList& paint_list, const WorkbenchTheme& theme) const {
  const Target* target = active_.has_value() ? find(*active_) : nullptr;
  if (!live_->visible || target == nullptr || live_->opacity <= 0.0F) return;
  const auto faded = [&](Color color) {
    color.a *= live_->opacity;
    return color;
  };
  const BorderRadii radius = BorderRadii::all(4.0F);
  paint_list.draw_box_shadow(
      bounds_,
      BoxShadow{.color = faded(theme.shadow), .offset = {0, 2}, .blur_radius = 8.0F},
      radius);
  paint_list.fill_stroked_rounded_rect(bounds_, faded(theme.tooltip), radius,
                                       faded(theme.border), 1.0F);
  paint_list.push_clip(bounds_);
  paint_list.fill_text(
      Rect{{bounds_.origin.x + options_.padding, bounds_.origin.y + options_.padding},
           {text_width_ + 1.0F, theme.font_size * 1.4F}},
      faded(theme.tooltip_text), target->text, theme.font, theme.font_size);
  paint_list.pop_clip();
}

} // namespace cgpui
