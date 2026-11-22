#include "cgpui/ui/tooltip.hpp"

#include "widget_popup_internal.hpp"

#include <memory>

namespace cgpui {

// The popup window adds a margin for the shadow (blur 8, offset 2) on
// every side; the platform places it against the screen, not the window.
void Tooltip::sync_popup(Rect anchor, Size size, const WorkbenchTheme& theme) {
  constexpr float margin = 16.0F;
  in_popup_ = false;
  if (!options_.popup_window || runtime_ == nullptr || !live_->visible) {
    if (popup_) popup_->close();
    return;
  }
  if (!popup_) popup_ = std::make_shared<workbench_detail::WidgetPopup>();
  popup_theme_ = std::make_shared<WorkbenchTheme>(theme);
  const float gap = options_.position == TooltipPosition::anchor ? options_.gap : 0.0F;
  const auto window = popup_->show(
      *runtime_,
      WindowPopupPlacement{
          .anchor = {{anchor.origin.x - margin, anchor.origin.y}, anchor.size},
          .side = WindowPopupSide::below,
          .gap = gap - margin},
      Size{size.width + margin * 2.0F, size.height + margin * 2.0F},
      WindowPopupOptions{.pointer_transparent = true},
      workbench_detail::WidgetPopupContent{
          .paint = [this, theme = popup_theme_](PaintList& paint_list) {
            paint_bubble(paint_list, *theme);
          }});
  if (!window) return;
  bounds_ = Rect{{window->origin.x + margin, window->origin.y + margin}, size};
  flipped_ = bounds_.origin.y < anchor.origin.y;
  in_popup_ = true;
}

} // namespace cgpui
