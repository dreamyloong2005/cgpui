#include "cgpui/ui/popover_menu.hpp"

#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/text_measurement.hpp"
#include "workbench_internal.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace cgpui {

MenuItem MenuItem::action(
    std::string id,
    std::string label,
    std::string accelerator,
    bool enabled) {
  return MenuItem{.kind = MenuItemKind::action,
                  .id = std::move(id),
                  .label = std::move(label),
                  .accelerator = std::move(accelerator),
                  .enabled = enabled};
}

MenuItem MenuItem::separator() {
  return MenuItem{.kind = MenuItemKind::separator};
}

MenuItem MenuItem::submenu(
    std::string label,
    std::vector<MenuItem> children,
    bool enabled) {
  return MenuItem{.kind = MenuItemKind::submenu,
                  .label = std::move(label),
                  .enabled = enabled,
                  .children = std::move(children)};
}

PopoverMenu::PopoverMenu(PopoverMenuOptions options) : options_(std::move(options)) {}

void PopoverMenu::open_at(
    Point position,
    std::vector<MenuItem> items,
    const WindowRuntimeContext& context) {
  if (options_.prefer_native_menu && open_native(position, items, context)) return;
  anchored_to_rect_ = false;
  position_ = position;
  open(std::move(items), context);
}

void PopoverMenu::open_beside(
    Rect anchor,
    AnchoredSide side,
    std::vector<MenuItem> items,
    const WindowRuntimeContext& context) {
  anchored_to_rect_ = true;
  anchor_ = anchor;
  side_ = side;
  open(std::move(items), context);
}

void PopoverMenu::open(std::vector<MenuItem> items, const WindowRuntimeContext& context) {
  if (!open_) {
    restore_focus_ = context.input_state().keyboard_focus_element_owner;
  }
  items_ = std::move(items);
  levels_.clear();
  levels_.push_back(Level{.items = &items_});
  open_ = true;
  focus_pending_ = true;
  pressed_inside_ = false;
  if (menu_id_.value != 0) context.request_keyboard_focus(menu_id_);
  place();
  context.request_render();
}

void PopoverMenu::close(PopoverCloseReason reason, const WindowRuntimeContext& context) {
  if (!open_) return;
  open_ = false;
  focus_pending_ = false;
  levels_.clear();
  if (restore_focus_.has_value()) {
    context.request_keyboard_focus(*restore_focus_);
  } else if (menu_id_.value != 0) {
    context.release_keyboard_focus(menu_id_);
  }
  restore_focus_.reset();
  context.request_render();
  if (close_handler_) close_handler_(reason);
}

bool PopoverMenu::is_open() const {
  return open_;
}

void PopoverMenu::on_activate(MenuActivateHandler handler) {
  activate_handler_ = std::move(handler);
}

void PopoverMenu::on_close(PopoverCloseHandler handler) {
  close_handler_ = std::move(handler);
}

void PopoverMenu::layout(Size viewport, DpiScale scale, const WorkbenchTheme& theme) {
  const bool font_changed = font_ != theme.font || font_size_ != theme.font_size ||
                            scale_.value != scale.value;
  viewport_ = viewport;
  scale_ = scale;
  font_ = theme.font;
  font_size_ = theme.font_size;
  if (font_changed) {
    for (Level& level : levels_) level.measured = false;
  }
  place();
}

// Real text metrics: every label and accelerator is shaped once when its
// level opens (or the font changes); the accelerator column starts after
// the widest label plus `accelerator_gap`.
void PopoverMenu::measure(Level& level) {
  const std::vector<MenuItem>& items = *level.items;
  level.label_widths.assign(items.size(), 0.0F);
  level.accelerator_widths.assign(items.size(), 0.0F);
  level.item_y.assign(items.size(), 0.0F);
  level.label_column = 0.0F;
  level.accelerator_column = 0.0F;
  level.has_submenu = false;
  float y = options_.padding;
  for (std::size_t index = 0; index < items.size(); ++index) {
    const MenuItem& item = items[index];
    level.item_y[index] = y;
    if (item.kind == MenuItemKind::separator) {
      y += options_.separator_height;
      continue;
    }
    y += options_.item_height;
    level.label_widths[index] =
        measure_text(item.label, font_, font_size_, scale_).logical_size.width;
    level.label_column = std::max(level.label_column, level.label_widths[index]);
    if (!item.accelerator.empty()) {
      level.accelerator_widths[index] =
          measure_text(item.accelerator, font_, font_size_, scale_).logical_size.width;
      level.accelerator_column =
          std::max(level.accelerator_column, level.accelerator_widths[index]);
    }
    level.has_submenu = level.has_submenu || item.kind == MenuItemKind::submenu;
  }
  const float factor = workbench_detail::scale_of(scale_);
  float width = options_.padding * 2.0F + options_.item_padding * 2.0F +
                level.label_column;
  if (level.accelerator_column > 0.0F) {
    width += options_.accelerator_gap + level.accelerator_column;
  }
  if (level.has_submenu) width += options_.submenu_arrow_width;
  width = std::ceil(std::max(width, options_.min_width) * factor) / factor;
  level.bounds.size = Size{width, std::ceil((y + options_.padding) * factor) / factor};
  level.measured = true;
}

void PopoverMenu::place() {
  for (std::size_t index = 0; index < levels_.size(); ++index) {
    Level& level = levels_[index];
    if (!level.measured) measure(level);
    AnchoredBounds placed;
    if (index == 0) {
      placed = anchored_to_rect_
          ? anchored_bounds_beside(anchor_, side_, level.bounds.size, viewport_,
                                   2.0F, options_.margin)
          : anchored_bounds(AnchoredPlacement{.position = position_,
                                              .margin = options_.margin},
                            level.bounds.size, viewport_);
    } else {
      // Submenus open beside their item, the first item level with it.
      const Level& parent = levels_[index - 1];
      const Rect item = item_rect(index - 1, *level.parent_index);
      const Rect anchor{{parent.bounds.origin.x, item.origin.y - options_.padding},
                        {parent.bounds.size.width, item.size.height}};
      placed = anchored_bounds_beside(anchor, AnchoredSide::right, level.bounds.size,
                                      viewport_, 0.0F, options_.margin);
    }
    level.bounds.origin = Point{workbench_detail::snap(placed.bounds.origin.x, scale_),
                                workbench_detail::snap(placed.bounds.origin.y, scale_)};
    level.flipped_x = placed.flipped_x;
    level.flipped_y = placed.flipped_y;
  }
}

void PopoverMenu::open_submenu(std::size_t level, std::size_t index) {
  truncate(level + 1);
  const MenuItem& item = (*levels_[level].items)[index];
  if (item.kind != MenuItemKind::submenu || !item.enabled) return;
  levels_.push_back(Level{.items = &item.children, .parent_index = index});
  place();
}

void PopoverMenu::truncate(std::size_t count) {
  if (levels_.size() > count) levels_.resize(count);
}

} // namespace cgpui
