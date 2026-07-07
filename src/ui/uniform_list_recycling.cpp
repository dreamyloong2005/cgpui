#include "cgpui/ui/uniform_list.hpp"

#include <algorithm>

namespace cgpui {
namespace {

void add_recycled_extent(Size& extent, Size size) {
  extent.width = std::max(extent.width, size.width);
  extent.height += size.height;
}

} // namespace

bool UniformListRecyclingWindow::empty() const {
  return retained_count == 0;
}

bool UniformListRecyclingWindow::retains(std::size_t index) const {
  return retained_range.contains(index);
}

bool UniformListRecyclingWindow::recycles(std::size_t index) const {
  return !retains(index);
}

UniformListRecyclingWindow calculate_uniform_list_recycling_window(
    UniformListVisibleRange visible_range,
    std::span<const UniformListItemMeasurementResult> measurements,
    std::size_t overscan_items) {
  UniformListRecyclingWindow window;
  if (measurements.empty() || visible_range.empty()) {
    window.recycled_after_count = measurements.size();
    for (const UniformListItemMeasurementResult& result : measurements) {
      add_recycled_extent(
          window.recycled_after_size,
          result.measurement.size);
    }
    return window;
  }

  const std::size_t total_count = measurements.size();
  const std::size_t visible_start =
      std::min(visible_range.start_index, total_count);
  const std::size_t visible_end =
      std::min(std::max(visible_range.end_index, visible_start), total_count);
  if (visible_start >= visible_end) {
    window.recycled_after_count = measurements.size();
    for (const UniformListItemMeasurementResult& result : measurements) {
      add_recycled_extent(
          window.recycled_after_size,
          result.measurement.size);
    }
    return window;
  }

  window.visible_count = visible_end - visible_start;
  window.retained_range.start_index =
      visible_start > overscan_items ? visible_start - overscan_items : 0;
  window.retained_range.end_index =
      std::min(total_count, visible_end + overscan_items);

  for (const UniformListItemMeasurementResult& result : measurements) {
    const std::size_t index = result.measurement.index;
    if (index < window.retained_range.start_index) {
      window.recycled_before_count += 1;
      add_recycled_extent(
          window.recycled_before_size,
          result.measurement.size);
      continue;
    }
    if (index >= window.retained_range.end_index) {
      window.recycled_after_count += 1;
      add_recycled_extent(
          window.recycled_after_size,
          result.measurement.size);
      continue;
    }
    window.retained_count += 1;
  }

  return window;
}

} // namespace cgpui
