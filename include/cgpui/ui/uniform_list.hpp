#pragma once

#include "cgpui/ui/element_core.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

struct UniformListVisibleRange {
  std::size_t start_index = 0;
  std::size_t end_index = 0;

  [[nodiscard]] bool empty() const;
  [[nodiscard]] bool contains(std::size_t index) const;
};

struct UniformListItemIdentity {
  std::size_t index = 0;
  ElementKey key;
  ElementId element_id;
  Rect content_bounds;
  bool visible = false;
};

struct UniformListLayoutSnapshot {
  std::vector<UniformListItemIdentity> items;
  UniformListVisibleRange visible_range;
};

[[nodiscard]] UniformListVisibleRange calculate_uniform_list_visible_range(
    std::span<const UniformListItemIdentity> items,
    Point scroll_offset,
    Size viewport_size);

} // namespace cgpui
