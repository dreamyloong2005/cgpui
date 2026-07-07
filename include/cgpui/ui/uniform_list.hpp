#pragma once

#include "cgpui/ui/element_core.hpp"

#include <cstddef>
#include <optional>
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

struct UniformListItemMeasurement {
  std::size_t index = 0;
  ElementKey key;
  ElementId element_id;
  Size size;
};

struct UniformListItemMeasurementResult {
  UniformListItemMeasurement measurement;
  bool cache_hit = false;
};

struct UniformListLayoutSnapshot {
  std::vector<UniformListItemIdentity> items;
  std::vector<UniformListItemMeasurementResult> measurements;
  UniformListVisibleRange visible_range;
};

struct UniformListScrollAnchor {
  std::size_t index = 0;
  ElementKey key;
  ElementId element_id;
  float viewport_offset_y = 0.0F;
};

class UniformListItemMeasurementCache {
 public:
  [[nodiscard]] UniformListItemMeasurementResult measure(
      const UniformListItemIdentity& item);
  [[nodiscard]] std::optional<UniformListItemMeasurement> measurement_for(
      const ElementKey& key) const;

  [[nodiscard]] std::size_t entry_count() const;
  [[nodiscard]] std::size_t lookup_count() const;
  [[nodiscard]] std::size_t hit_count() const;
  [[nodiscard]] std::size_t miss_count() const;

  void clear();

 private:
  struct Entry {
    UniformListItemMeasurement measurement;
  };

  std::vector<Entry> entries_;
  std::size_t lookup_count_ = 0;
  std::size_t hit_count_ = 0;
  std::size_t miss_count_ = 0;
};

[[nodiscard]] UniformListVisibleRange calculate_uniform_list_visible_range(
    std::span<const UniformListItemIdentity> items,
    Point scroll_offset,
    Size viewport_size);

[[nodiscard]] std::vector<UniformListItemMeasurementResult>
measure_uniform_list_items(
    UniformListItemMeasurementCache& cache,
    std::span<const UniformListItemIdentity> items);

[[nodiscard]] std::optional<UniformListScrollAnchor>
capture_uniform_list_scroll_anchor(
    std::span<const UniformListItemIdentity> items,
    Point scroll_offset);

[[nodiscard]] Point apply_uniform_list_scroll_anchor(
    std::span<const UniformListItemIdentity> items,
    const UniformListScrollAnchor& anchor,
    Point scroll_offset);

} // namespace cgpui
