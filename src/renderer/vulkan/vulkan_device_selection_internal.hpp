#pragma once

#include "vulkan_platform_internal.hpp"
#include <optional>
#include <string_view>

namespace cgpui {
Result<std::optional<std::uint32_t>> vulkan_parse_device_selection(std::string_view value);
Result<void> vulkan_filter_device_candidates(std::vector<VkPhysicalDevice>& devices,
    std::optional<std::uint32_t> requested_index, VkPhysicalDevice preferred);
} // namespace cgpui
