#include "vulkan_glyph_atlas_validation_internal.hpp"

#include <cmath>

namespace cgpui {

Result<VkExtent2D> vulkan_glyph_atlas_extent(Size size, std::uint32_t maximum_dimension) {
  // Compare in double before integer conversion: float(UINT32_MAX) rounds up.
  const double width = std::ceil(static_cast<double>(size.width));
  const double height = std::ceil(static_cast<double>(size.height));
  if (!std::isfinite(width) || !std::isfinite(height) || width <= 0 || height <= 0 ||
      width > maximum_dimension || height > maximum_dimension) {
    return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
        "glyph atlas extent exceeds the current Vulkan device limit"));
  }
  return VkExtent2D{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
}

Result<void> vulkan_validate_glyph_atlas_resources(
    VkPhysicalDevice device, std::span<const GlyphAtlasProductionResourceRecord> records) {
  if (records.empty()) return {};
  if (device == VK_NULL_HANDLE) return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed, "glyph atlas validation requires a physical device"));
  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(device, &properties);
  for (const auto& record : records) {
    if (auto extent = vulkan_glyph_atlas_extent(record.image.size, properties.limits.maxImageDimension2D);
        !extent) return std::unexpected(extent.error());
  }
  return {};
}

} // namespace cgpui
