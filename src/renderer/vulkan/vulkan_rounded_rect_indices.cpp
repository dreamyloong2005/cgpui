#include "vulkan_rounded_rect_indices_internal.hpp"

namespace cgpui {

void vulkan_append_rounded_rect_fan_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t center,
    std::uint32_t first_ring,
    std::size_t perimeter_count) {
  for (std::size_t index = 0; index < perimeter_count; ++index) {
    const auto current = static_cast<std::uint32_t>(index);
    const auto next = static_cast<std::uint32_t>((index + 1) % perimeter_count);
    indices.insert(
        indices.end(), {center, first_ring + current, first_ring + next});
  }
}

void vulkan_append_rounded_rect_ring_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t first_inner,
    std::uint32_t first_outer,
    std::size_t perimeter_count) {
  for (std::size_t index = 0; index < perimeter_count; ++index) {
    const auto current = static_cast<std::uint32_t>(index);
    const auto next = static_cast<std::uint32_t>((index + 1) % perimeter_count);
    indices.insert(
        indices.end(),
        {first_inner + current,
         first_outer + current,
         first_outer + next,
         first_inner + current,
         first_outer + next,
         first_inner + next});
  }
}

} // namespace cgpui
