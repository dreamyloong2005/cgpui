#include "cgpui/ui/uniform_list.hpp"

#include <algorithm>

namespace cgpui {
namespace {

[[nodiscard]] bool intersects_viewport(
    Rect bounds,
    Point scroll_offset,
    Size viewport_size) {
  if (viewport_size.width <= 0.0F || viewport_size.height <= 0.0F ||
      bounds.size.width <= 0.0F || bounds.size.height <= 0.0F) {
    return false;
  }

  const float viewport_left = scroll_offset.x;
  const float viewport_top = scroll_offset.y;
  const float viewport_right = viewport_left + viewport_size.width;
  const float viewport_bottom = viewport_top + viewport_size.height;
  const float item_left = bounds.origin.x;
  const float item_top = bounds.origin.y;
  const float item_right = item_left + bounds.size.width;
  const float item_bottom = item_top + bounds.size.height;
  return item_left < viewport_right && item_right > viewport_left &&
         item_top < viewport_bottom && item_bottom > viewport_top;
}

} // namespace

bool UniformListVisibleRange::empty() const {
  return start_index >= end_index;
}

bool UniformListVisibleRange::contains(std::size_t index) const {
  return index >= start_index && index < end_index;
}

UniformListVisibleRange calculate_uniform_list_visible_range(
    std::span<const UniformListItemIdentity> items,
    Point scroll_offset,
    Size viewport_size) {
  UniformListVisibleRange range;
  bool found_visible = false;
  for (const UniformListItemIdentity& item : items) {
    if (!intersects_viewport(item.content_bounds, scroll_offset, viewport_size)) {
      continue;
    }
    if (!found_visible) {
      range.start_index = item.index;
      range.end_index = item.index + 1;
      found_visible = true;
      continue;
    }
    range.start_index = std::min(range.start_index, item.index);
    range.end_index = std::max(range.end_index, item.index + 1);
  }
  return found_visible ? range : UniformListVisibleRange{};
}

std::optional<UniformListScrollAnchor> capture_uniform_list_scroll_anchor(
    std::span<const UniformListItemIdentity> items,
    Point scroll_offset) {
  for (const UniformListItemIdentity& item : items) {
    if (item.content_bounds.size.height <= 0.0F) {
      continue;
    }
    const float item_bottom =
        item.content_bounds.origin.y + item.content_bounds.size.height;
    if (item_bottom <= scroll_offset.y) {
      continue;
    }
    return UniformListScrollAnchor{
        .index = item.index,
        .key = item.key,
        .element_id = item.element_id,
        .viewport_offset_y = item.content_bounds.origin.y - scroll_offset.y,
    };
  }
  return std::nullopt;
}

Point apply_uniform_list_scroll_anchor(
    std::span<const UniformListItemIdentity> items,
    const UniformListScrollAnchor& anchor,
    Point scroll_offset) {
  for (const UniformListItemIdentity& item : items) {
    if (item.key == anchor.key) {
      scroll_offset.y = item.content_bounds.origin.y - anchor.viewport_offset_y;
      return scroll_offset;
    }
  }
  return scroll_offset;
}

} // namespace cgpui
