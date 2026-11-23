#include "vulkan_rounded_rect_indices_internal.hpp"
#include "../planning/rounded_rect_indices_internal.hpp"

namespace cgpui {

void vulkan_append_rounded_rect_fan_indices(std::vector<std::uint32_t>& indices,
    std::uint32_t center, std::uint32_t first_ring, std::size_t count) {
  append_rounded_rect_fan_indices(indices, center, first_ring, count);
}
void vulkan_append_rounded_rect_ring_indices(std::vector<std::uint32_t>& indices,
    std::uint32_t first_inner, std::uint32_t first_outer, std::size_t count) {
  append_rounded_rect_ring_indices(indices, first_inner, first_outer, count);
}

} // namespace cgpui
