#include "cgpui/ui/tree_view.hpp"

#include "workbench_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

void TreeView::layout(Rect bounds, DpiScale scale) {
  bounds_ = bounds;
  scale_ = scale;
  scroll_ = std::clamp(scroll_, 0.0F, max_scroll());
}

float TreeView::max_scroll() const {
  return std::max(0.0F, static_cast<float>(row_count()) * options_.row_height -
                            bounds_.size.height);
}

std::size_t TreeView::page_rows() const {
  return std::max<std::size_t>(
      1, static_cast<std::size_t>(bounds_.size.height / options_.row_height));
}

std::optional<std::size_t> TreeView::row_at(Point point) const {
  if (!cgpui::contains(bounds_, point)) return std::nullopt;
  const float offset = point.y - bounds_.origin.y + scroll_;
  const auto row = static_cast<std::size_t>(std::floor(offset / options_.row_height));
  if (row >= row_count()) return std::nullopt;
  return row;
}

// Rows sit on device pixels: the scroll offset is snapped, row height is a
// whole number of logical pixels in the defaults.
Rect TreeView::row_rect(std::size_t row) const {
  const float y = workbench_detail::snap(
      bounds_.origin.y + static_cast<float>(row) * options_.row_height - scroll_,
      scale_);
  return Rect{{bounds_.origin.x, y}, {bounds_.size.width, options_.row_height}};
}

// Rows intersecting the viewport, widened by `overscan` on both sides. The
// result is a uniform_list range: [start_index, end_index).
UniformListVisibleRange TreeView::visible_range() const {
  const std::size_t count = row_count();
  if (count == 0 || bounds_.size.height <= 0.0F) return {};
  const auto first = static_cast<std::size_t>(scroll_ / options_.row_height);
  const auto last = static_cast<std::size_t>(
      std::ceil((scroll_ + bounds_.size.height) / options_.row_height));
  return UniformListVisibleRange{
      .start_index = first > options_.overscan ? first - options_.overscan : 0,
      .end_index = std::min(count, last + options_.overscan),
  };
}

float TreeView::scroll_offset() const {
  return scroll_;
}

void TreeView::scroll_to(float offset) {
  scroll_ = std::clamp(offset, 0.0F, max_scroll());
}

void TreeView::scroll_into_view(TreeItemId item) {
  const std::optional<std::size_t> row = row_of(item);
  if (!row.has_value()) return;
  const float top = static_cast<float>(*row) * options_.row_height;
  const float bottom = top + options_.row_height;
  if (top < scroll_) {
    scroll_to(top);
  } else if (bottom > scroll_ + bounds_.size.height) {
    scroll_to(bottom - bounds_.size.height);
  }
}

bool TreeView::focused() const {
  return focused_;
}

ElementId TreeView::element_id() const {
  return tree_id_;
}

void TreeView::focus_row(std::size_t row, TreeSelectMode mode) {
  const std::optional<TreeItemId> item = item_at_row(row);
  if (!item.has_value()) return;
  select(*item, mode);
  scroll_into_view(*item);
}

} // namespace cgpui
