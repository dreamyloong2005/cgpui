#include "cgpui/ui/tab_strip.hpp"

#include "cgpui/ui/paint.hpp"
#include "workbench_internal.hpp"

#include <algorithm>

namespace cgpui {

void TabStrip::append_elements(
    WidgetTreeBuilder& builder,
    WidgetTreeBuilder::NodeHandle parent) {
  const std::optional<ElementId> focused = builder.focused_element();
  focused_ = focused.has_value() && strip_id_.value != 0 && *focused == strip_id_;
  const WidgetTreeBuilder::NodeHandle strip = builder.add(
      parent,
      WidgetNode{.role = AccessibilityRole::tab_list,
                 .name = options_.name,
                 .bounds = bounds_,
                 .focusable = true,
                 .hit_testable = true,
                 .z_order = 5},
      true,
      &strip_id_);
  for (std::size_t index = 0; index < tabs_.size(); ++index) {
    const TabItem& tab = tabs_[index];
    const WidgetTreeBuilder::NodeHandle node = builder.add(
        strip,
        WidgetNode{.role = AccessibilityRole::tab,
                   .name = tab.title,
                   .value = tab.dirty ? "modified" : "",
                   .patterns = {.selected = selected_ == tab.id},
                   .bounds = tab_rect(index)});
    if (tab.closable) {
      builder.add(node, WidgetNode{.role = AccessibilityRole::button,
                                   .name = "Close " + tab.title,
                                   .bounds = close_rect(index)});
    }
  }
}

namespace {

// Vertically centers one line of text in `area` (line box ~1.4 em).
Rect text_box(Rect area, float x, float width, float font_size) {
  const float line = font_size * 1.4F;
  return Rect{{x, area.origin.y + (area.size.height - line) / 2.0F},
              {std::max(0.0F, width), line}};
}

} // namespace

void TabStrip::paint(PaintList& paint_list, const WorkbenchTheme& theme) const {
  using workbench_detail::right;
  paint_list.fill_rect(bounds_, theme.tab_strip);
  paint_list.push_clip(bounds_);
  const float hairline = workbench_detail::snap_length(1.0F, scale_);
  const auto paint_tab = [&](std::size_t index, float offset) {
    const TabItem& tab = tabs_[index];
    Rect rect = tab_rect(index);
    rect.origin.x += offset;
    const bool selected = selected_ == tab.id;
    const bool hovered = hovered_ == index;
    paint_list.fill_rect(rect, selected ? theme.tab_active : theme.tab_strip);
    if (hovered && !selected) paint_list.fill_rect(rect, theme.hover);
    paint_list.fill_rect(Rect{{right(rect) - hairline, rect.origin.y},
                              {hairline, rect.size.height}},
                         theme.border);
    if (selected) {
      paint_list.fill_rect(Rect{rect.origin, {rect.size.width, 2.0F * hairline}},
                           theme.accent);
    }
    Rect close = close_rect(index);
    close.origin.x += offset;
    const Rect title_area{rect.origin,
                          {close.origin.x - 4.0F - rect.origin.x, rect.size.height}};
    paint_list.push_clip(title_area);
    // Width is the measured title width (+1 px) so the title never wraps;
    // the clip cuts titles longer than the tab.
    paint_list.fill_text(
        text_box(rect, rect.origin.x + options_.padding, title_width(index) + 1.0F,
                 theme.font_size),
        selected ? theme.text : theme.text_muted, tab.title, theme.font,
        theme.font_size);
    paint_list.pop_clip();
    const bool show_close = tab.closable && (selected || hovered);
    if (tab.dirty && !(hovered && tab.closable)) {
      const float dot = 8.0F;
      paint_list.fill_rounded_rect(
          Rect{{close.origin.x + (close.size.width - dot) / 2.0F,
                close.origin.y + (close.size.height - dot) / 2.0F},
               {dot, dot}},
          theme.dirty, BorderRadii::all(dot / 2.0F));
    } else if (show_close) {
      if (hovered && hover_close_) {
        paint_list.fill_rounded_rect(close, theme.hover, BorderRadii::all(4.0F));
      }
      // U+00D7 MULTIPLICATION SIGN.
      paint_list.fill_text(text_box(close, close.origin.x + 4.0F, close.size.width,
                                    theme.font_size + 2.0F),
                           theme.text_muted, "\xC3\x97", theme.font,
                           theme.font_size + 2.0F);
    }
    if (selected && focused_) {
      paint_list.stroke_rounded_rect(
          Rect{{rect.origin.x + 1.0F, rect.origin.y + 2.0F},
               {rect.size.width - 2.0F, rect.size.height - 3.0F}},
          theme.focus_ring, BorderRadii::all(2.0F), theme.focus_ring_width);
    }
  };
  for (std::size_t index = 0; index < tabs_.size(); ++index) {
    if (dragging_ && pressed_ == index) continue;
    paint_tab(index, 0.0F);
  }
  if (dragging_ && pressed_.has_value() && *pressed_ < tabs_.size()) {
    // Insertion marker where the dragged tab would land.
    const std::size_t to = drop_index_for(drag_x_);
    float marker = bounds_.origin.x - scroll_;
    std::size_t placed = 0;
    for (std::size_t index = 0; index < tabs_.size() && placed < to; ++index) {
      if (index == *pressed_) continue;
      marker = right(tab_rect(index));
      placed += 1;
    }
    paint_list.fill_rect(Rect{{marker - hairline, bounds_.origin.y},
                              {2.0F * hairline, bounds_.size.height}},
                         theme.accent);
    const Rect dragged = tab_rect(*pressed_);
    const float offset = drag_x_ - press_x_;
    paint_list.draw_box_shadow(
        Rect{{dragged.origin.x + offset, dragged.origin.y}, dragged.size},
        BoxShadow{.color = theme.shadow, .offset = {0, 2}, .blur_radius = 8.0F});
    paint_tab(*pressed_, offset);
  }
  paint_list.pop_clip();
  paint_list.fill_rect(
      Rect{{bounds_.origin.x, workbench_detail::bottom(bounds_) - hairline},
           {bounds_.size.width, hairline}},
      theme.border);
}

} // namespace cgpui
