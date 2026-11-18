#include "cgpui/ui/popover_menu.hpp"

#include "workbench_internal.hpp"

namespace cgpui {

std::size_t PopoverMenu::level_count() const {
  return levels_.size();
}

Rect PopoverMenu::level_bounds(std::size_t level) const {
  return level < levels_.size() ? levels_[level].bounds : Rect{};
}

Rect PopoverMenu::item_rect(std::size_t level, std::size_t index) const {
  if (level >= levels_.size() || index >= levels_[level].item_y.size()) return {};
  const Level& entry = levels_[level];
  const bool separator = (*entry.items)[index].kind == MenuItemKind::separator;
  return Rect{{entry.bounds.origin.x + options_.padding,
               entry.bounds.origin.y + entry.item_y[index]},
              {entry.bounds.size.width - options_.padding * 2.0F,
               separator ? options_.separator_height : options_.item_height}};
}

std::optional<std::size_t> PopoverMenu::highlighted(std::size_t level) const {
  return level < levels_.size() ? levels_[level].highlighted : std::nullopt;
}

float PopoverMenu::label_column_width(std::size_t level) const {
  return level < levels_.size() ? levels_[level].label_column : 0.0F;
}

// Left edge of the accelerator column (accelerators are right-aligned in it).
float PopoverMenu::accelerator_column_x(std::size_t level) const {
  if (level >= levels_.size()) return 0.0F;
  const Level& entry = levels_[level];
  const float arrow = entry.has_submenu ? options_.submenu_arrow_width : 0.0F;
  return workbench_detail::right(entry.bounds) - options_.padding -
         options_.item_padding - arrow - entry.accelerator_column;
}

float PopoverMenu::accelerator_width(std::size_t level, std::size_t index) const {
  if (level >= levels_.size() || index >= levels_[level].accelerator_widths.size()) {
    return 0.0F;
  }
  return levels_[level].accelerator_widths[index];
}

bool PopoverMenu::flipped_x(std::size_t level) const {
  return level < levels_.size() && levels_[level].flipped_x;
}

bool PopoverMenu::flipped_y(std::size_t level) const {
  return level < levels_.size() && levels_[level].flipped_y;
}

ElementId PopoverMenu::element_id() const {
  return menu_id_;
}

} // namespace cgpui
