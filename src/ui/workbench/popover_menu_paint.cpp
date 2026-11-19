#include "cgpui/ui/popover_menu.hpp"

#include "cgpui/ui/paint.hpp"
#include "workbench_internal.hpp"

#include <algorithm>

namespace cgpui {

void PopoverMenu::append_elements(
    WidgetTreeBuilder& builder,
    WidgetTreeBuilder::NodeHandle parent) {
  // Always present (stable id). While open it covers the window: it is the
  // focus target and takes every pointer hit, which is how outside clicks
  // reach the menu instead of the content underneath.
  WidgetNode root;
  if (open_) {
    root = WidgetNode{.role = AccessibilityRole::menu,
                      .name = options_.name,
                      .bounds = Rect{.size = builder.viewport()},
                      .focusable = true,
                      .hit_testable = true,
                      .z_order = 1000};
  }
  WidgetTreeBuilder::NodeHandle container =
      builder.add(parent, std::move(root), true, &menu_id_);
  if (focus_pending_) {
    builder.request_focus(&menu_id_);
    focus_pending_ = false;
  }
  for (std::size_t level = 0; level < levels_.size(); ++level) {
    const Level& entry = levels_[level];
    WidgetTreeBuilder::NodeHandle next = container;
    for (std::size_t index = 0; index < entry.items->size(); ++index) {
      const MenuItem& item = (*entry.items)[index];
      if (item.kind == MenuItemKind::separator) {
        builder.add(container, WidgetNode{.role = AccessibilityRole::separator,
                                          .bounds = item_rect(level, index)});
        continue;
      }
      AccessibilityPatternState patterns{
          .invokable = item.enabled,
          .selected = entry.highlighted == index};
      const bool open_child = level + 1 < levels_.size() &&
                              levels_[level + 1].parent_index == index;
      if (item.kind == MenuItemKind::submenu) patterns.expanded = open_child;
      const WidgetTreeBuilder::NodeHandle node = builder.add(
          container,
          WidgetNode{.role = AccessibilityRole::menu_item,
                     .name = item.label,
                     .value = item.accelerator,
                     .patterns = patterns,
                     .bounds = item_rect(level, index),
                     .enabled = item.enabled});
      if (open_child) next = node;
    }
    if (level + 1 < levels_.size()) {
      const Level& child = levels_[level + 1];
      container = builder.add(
          next, WidgetNode{.role = AccessibilityRole::menu,
                           .name = (*entry.items)[*child.parent_index].label,
                           .bounds = child.bounds});
    }
  }
}

namespace {

Rect line_box(Rect item, float x, float width, float font_size) {
  const float line = font_size * 1.4F;
  return Rect{{x, item.origin.y + (item.size.height - line) / 2.0F},
              {width + 1.0F, line}};
}

} // namespace

void PopoverMenu::paint(PaintList& paint_list, const WorkbenchTheme& theme) const {
  const BorderRadii radius = BorderRadii::all(options_.corner_radius);
  const float hairline = workbench_detail::snap_length(1.0F, scale_);
  for (std::size_t level = 0; level < levels_.size(); ++level) {
    const Level& entry = levels_[level];
    const Rect bounds = entry.bounds;
    // C5 shadow under the surface; optional C6 blur of what lies beneath.
    paint_list.draw_box_shadow(
        bounds,
        BoxShadow{.color = theme.shadow, .offset = {0, 8}, .blur_radius = 24.0F},
        radius);
    // Opaque unless a backdrop blur is requested: content under the menu
    // must not show through a plain surface.
    Color surface = theme.menu;
    surface.a = 1.0F;
    if (options_.backdrop_blur) {
      paint_list.draw_backdrop_blur(bounds, radius, options_.blur_radius);
      surface.a = std::min(theme.menu.a, 0.78F);  // let the blur show through
    }
    paint_list.fill_stroked_rounded_rect(bounds, surface, radius,
                                         theme.menu_border, hairline);
    const float accelerator_right = accelerator_column_x(level) + entry.accelerator_column;
    for (std::size_t index = 0; index < entry.items->size(); ++index) {
      const MenuItem& item = (*entry.items)[index];
      const Rect rect = item_rect(level, index);
      if (item.kind == MenuItemKind::separator) {
        paint_list.fill_rect(
            Rect{{rect.origin.x + options_.item_padding / 2.0F,
                  workbench_detail::snap(rect.origin.y + rect.size.height / 2.0F, scale_)},
                 {rect.size.width - options_.item_padding, hairline}},
            theme.border);
        continue;
      }
      const bool highlighted = entry.highlighted == index;
      if (highlighted) {
        paint_list.fill_rounded_rect(rect, theme.menu_highlight, BorderRadii::all(4.0F));
      }
      const Color color = !item.enabled ? theme.text_disabled
                          : highlighted ? theme.menu_highlight_text
                                        : theme.text;
      paint_list.fill_text(
          line_box(rect, rect.origin.x + options_.item_padding,
                   entry.label_widths[index], font_size_),
          color, item.label, font_, font_size_);
      if (!item.accelerator.empty()) {
        // Right-aligned in the accelerator column.
        const float width = entry.accelerator_widths[index];
        paint_list.fill_text(
            line_box(rect, accelerator_right - width, width, font_size_),
            highlighted ? theme.menu_highlight_text : theme.text_muted,
            item.accelerator, font_, font_size_);
      }
      if (item.kind == MenuItemKind::submenu) {
        // U+203A SINGLE RIGHT-POINTING ANGLE QUOTATION MARK.
        paint_list.fill_text(
            line_box(rect, workbench_detail::right(rect) - options_.item_padding -
                               options_.submenu_arrow_width / 2.0F,
                     options_.submenu_arrow_width, font_size_ + 2.0F),
            color, "\xE2\x80\xBA", font_, font_size_ + 2.0F);
      }
    }
  }
}

} // namespace cgpui
