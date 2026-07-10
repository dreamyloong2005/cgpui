#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace cgpui {

void vulkan_append_rounded_rect_fan_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t center,
    std::uint32_t first_ring,
    std::size_t perimeter_count);

void vulkan_append_rounded_rect_ring_indices(
    std::vector<std::uint32_t>& indices,
    std::uint32_t first_inner,
    std::uint32_t first_outer,
    std::size_t perimeter_count);

} // namespace cgpui
