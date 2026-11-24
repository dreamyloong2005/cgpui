#pragma once

#include "vulkan_glyph_atlas_resources_internal.hpp"

namespace cgpui {

Result<VkExtent2D> vulkan_glyph_atlas_extent(Size size, std::uint32_t maximum_dimension);
Result<void> vulkan_validate_glyph_atlas_resources(
    VkPhysicalDevice device, std::span<const GlyphAtlasProductionResourceRecord> records);

} // namespace cgpui
