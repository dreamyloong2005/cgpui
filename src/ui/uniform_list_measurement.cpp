#include "cgpui/ui/uniform_list.hpp"

namespace cgpui {
namespace {

[[nodiscard]] UniformListItemMeasurement measurement_from_item(
    const UniformListItemIdentity& item) {
  return UniformListItemMeasurement{
      .index = item.index,
      .key = item.key,
      .element_id = item.element_id,
      .size = item.content_bounds.size,
  };
}

} // namespace

UniformListItemMeasurementResult UniformListItemMeasurementCache::measure(
    const UniformListItemIdentity& item) {
  const UniformListItemMeasurement measurement = measurement_from_item(item);
  lookup_count_ += 1;
  for (Entry& entry : entries_) {
    if (entry.measurement.key == measurement.key) {
      hit_count_ += 1;
      entry.measurement = measurement;
      return UniformListItemMeasurementResult{
          .measurement = measurement,
          .cache_hit = true,
      };
    }
  }

  miss_count_ += 1;
  entries_.push_back(Entry{.measurement = measurement});
  return UniformListItemMeasurementResult{
      .measurement = measurement,
      .cache_hit = false,
  };
}

std::optional<UniformListItemMeasurement>
UniformListItemMeasurementCache::measurement_for(
    const ElementKey& key) const {
  for (const Entry& entry : entries_) {
    if (entry.measurement.key == key) {
      return entry.measurement;
    }
  }
  return std::nullopt;
}

std::size_t UniformListItemMeasurementCache::entry_count() const {
  return entries_.size();
}

std::size_t UniformListItemMeasurementCache::lookup_count() const {
  return lookup_count_;
}

std::size_t UniformListItemMeasurementCache::hit_count() const {
  return hit_count_;
}

std::size_t UniformListItemMeasurementCache::miss_count() const {
  return miss_count_;
}

void UniformListItemMeasurementCache::clear() {
  entries_.clear();
}

std::vector<UniformListItemMeasurementResult> measure_uniform_list_items(
    UniformListItemMeasurementCache& cache,
    std::span<const UniformListItemIdentity> items) {
  std::vector<UniformListItemMeasurementResult> measurements;
  measurements.reserve(items.size());
  for (const UniformListItemIdentity& item : items) {
    measurements.push_back(cache.measure(item));
  }
  return measurements;
}

} // namespace cgpui
