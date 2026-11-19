#include "cgpui/ui/tree_view.hpp"

#include "cgpui/ui/paint.hpp"
#include "workbench_internal.hpp"

#include <algorithm>

namespace cgpui {

void TreeView::append_elements(
    WidgetTreeBuilder& builder,
    WidgetTreeBuilder::NodeHandle parent) {
  const std::optional<ElementId> focused = builder.focused_element();
  focused_ = focused.has_value() && tree_id_.value != 0 && *focused == tree_id_;
  const WidgetTreeBuilder::NodeHandle tree = builder.add(
      parent,
      WidgetNode{.role = AccessibilityRole::tree,
                 .name = options_.name,
                 .bounds = bounds_,
                 .focusable = true,
                 .hit_testable = true},
      true,
      &tree_id_);
  // Virtualized: only rows in the visible range get nodes.
  const UniformListVisibleRange range = visible_range();
  for (std::size_t row = range.start_index; row < range.end_index; ++row) {
    const TreeItemId item = rows_[row];
    const Node& node = nodes_.at(item);
    AccessibilityPatternState patterns{
        .selected = selected_.contains(item),
        .level = static_cast<int>(node.depth) + 1,
    };
    if (node.has_children) patterns.expanded = node.expanded;
    builder.add(tree, WidgetNode{.role = AccessibilityRole::tree_item,
                                 .name = node.label,
                                 .patterns = patterns,
                                 .bounds = row_rect(row),
                                 .on_action = [this, item](const auto& action) {
                                   return handle_item_action(item, action.kind);
                                 }});
  }
}

namespace {

Rect line_box(Rect row, float x, float width, float font_size) {
  const float line = font_size * 1.4F;
  return Rect{{x, row.origin.y + (row.size.height - line) / 2.0F},
              {std::max(1.0F, width), line}};
}

} // namespace

void TreeView::paint(PaintList& paint_list, const WorkbenchTheme& theme) const {
  paint_list.fill_rect(bounds_, theme.surface_alt);
  paint_list.push_clip(bounds_);
  const float hairline = workbench_detail::snap_length(1.0F, scale_);
  const UniformListVisibleRange range = visible_range();
  for (std::size_t row = range.start_index; row < range.end_index; ++row) {
    const TreeItemId item = rows_[row];
    const Node& node = nodes_.at(item);
    const Rect rect = row_rect(row);
    if (selected_.contains(item)) {
      paint_list.fill_rect(rect, focused_ ? theme.selection : theme.selection_inactive);
    } else if (hovered_row_ == row) {
      paint_list.fill_rect(rect, theme.hover);
    }
    if (focused_ && focused_item_ == item) {
      paint_list.stroke_rounded_rect(
          Rect{{rect.origin.x + 1.0F, rect.origin.y},
               {rect.size.width - 2.0F, rect.size.height}},
          theme.focus_ring, BorderRadii{}, theme.focus_ring_width);
    }
    const float x0 = bounds_.origin.x + 4.0F;
    if (options_.indent_guides) {
      for (std::uint32_t level = 0; level < node.depth; ++level) {
        const float x = workbench_detail::snap(
            x0 + static_cast<float>(level) * options_.indent + options_.indent / 2.0F,
            scale_);
        paint_list.fill_rect(Rect{{x, rect.origin.y}, {hairline, rect.size.height}},
                             theme.indent_guide);
      }
    }
    const float twisty = x0 + static_cast<float>(node.depth) * options_.indent;
    if (node.has_children) {
      // U+25BE / U+25B8 small down / right triangles.
      paint_list.fill_text(
          line_box(rect, twisty + 3.0F, options_.indent, theme.font_size - 2.0F),
          theme.text_muted, node.expanded ? "\xE2\x96\xBE" : "\xE2\x96\xB8",
          theme.font, theme.font_size - 2.0F);
    }
    // A wide box keeps labels on one line (fill_text wraps to its box);
    // the tree clip cuts what does not fit.
    const float label_x = twisty + options_.indent + 2.0F;
    paint_list.fill_text(
        line_box(rect, label_x, 4096.0F, theme.font_size),
        theme.text, node.label, theme.font, theme.font_size);
  }
  paint_list.pop_clip();
}

} // namespace cgpui
