#include "cgpui/ui/popover_menu.hpp"

#include "widget_popup_internal.hpp"

#include <utility>

namespace cgpui {
namespace {

// Room for the surface shadow (blur 24, offset 8) around each level.
constexpr float popup_margin = 40.0F;

WindowPopupSide popup_side(AnchoredSide side) {
  switch (side) {
    case AnchoredSide::above: return WindowPopupSide::above;
    case AnchoredSide::right: return WindowPopupSide::right;
    case AnchoredSide::left: return WindowPopupSide::left;
    case AnchoredSide::below: break;
  }
  return WindowPopupSide::below;
}

// The window is the surface plus the margin on every side: shift the
// anchor along the aligned edge and pull the gap in by the margin.
WindowPopupPlacement window_placement(Rect anchor, WindowPopupSide side, float gap) {
  const bool vertical = side == WindowPopupSide::below || side == WindowPopupSide::above;
  Rect shifted = anchor;
  if (vertical) shifted.origin.x -= popup_margin;
  else shifted.origin.y -= popup_margin;
  return {.anchor = shifted, .side = side, .gap = gap - popup_margin};
}

}  // namespace

// Each open level gets a popup window that the platform places against the
// screen: level 0 at the anchor or point, submenus beside their item. The
// levels keep root-window coordinates (possibly outside the window), so
// hit testing, keyboard handling and accessibility are unchanged; popup
// pointer input comes back through the root window.
void PopoverMenu::sync_popups() {
  const bool wanted = options_.popup_window && runtime_ != nullptr && open_ &&
                      !levels_.empty();
  if (popups_.size() > (wanted ? levels_.size() : 0)) {
    for (std::size_t index = wanted ? levels_.size() : 0; index < popups_.size(); ++index) {
      popups_[index]->close();
    }
    popups_.resize(wanted ? levels_.size() : 0);
  }
  in_popup_ = false;
  if (!wanted) return;
  const std::vector<Level> overlay = levels_;
  if (!popup_theme_) popup_theme_ = std::make_shared<WorkbenchTheme>();  // before layout
  for (std::size_t index = 0; index < levels_.size(); ++index) {
    if (popups_.size() <= index) {
      popups_.push_back(std::make_shared<workbench_detail::WidgetPopup>());
    }
    Level& level = levels_[index];
    WindowPopupPlacement placement;
    if (index == 0) {
      placement = anchored_to_rect_
          ? window_placement(anchor_, popup_side(side_), 2.0F)
          : window_placement(Rect{position_, {}}, WindowPopupSide::below, 0.0F);
    } else {
      const Level& parent = levels_[index - 1];
      const Rect item = item_rect(index - 1, *level.parent_index);
      placement = window_placement(
          Rect{{parent.bounds.origin.x, item.origin.y - options_.padding},
               {parent.bounds.size.width, item.size.height}},
          WindowPopupSide::right, 0.0F);
    }
    const auto window = popups_[index]->show(
        *runtime_, placement,
        Size{level.bounds.size.width + popup_margin * 2.0F,
             level.bounds.size.height + popup_margin * 2.0F},
        WindowPopupOptions{},
        workbench_detail::WidgetPopupContent{
            .paint = [this, index, theme = popup_theme_](PaintList& paint_list) {
              if (index < levels_.size()) paint_level(paint_list, *theme, index, true);
            },
            .event = workbench_detail::forward_pointer_input_to_root()});
    if (!window) {
      // No popups here (or one failed): keep the in-window overlay.
      for (const auto& popup : popups_) popup->close();
      popups_.clear();
      levels_ = overlay;
      return;
    }
    const Point origin{window->origin.x + popup_margin, window->origin.y + popup_margin};
    level.flipped_x = origin.x < placement.anchor.origin.x;
    level.flipped_y = origin.y < placement.anchor.origin.y;
    level.bounds.origin = origin;
  }
  in_popup_ = true;
}

}  // namespace cgpui
