#include "vulkan_device_internal.hpp"

namespace cgpui {

Result<std::uint32_t> vulkan_find_memory_type(
    VkPhysicalDevice physical_device,
    std::uint32_t supported_types,
    VkMemoryPropertyFlags required_flags) {
  VkPhysicalDeviceMemoryProperties properties{};
  vkGetPhysicalDeviceMemoryProperties(physical_device, &properties);
  for (std::uint32_t index = 0; index < properties.memoryTypeCount; ++index) {
    const bool supported = (supported_types & (1U << index)) != 0;
    const bool has_flags =
        (properties.memoryTypes[index].propertyFlags & required_flags) ==
        required_flags;
    if (supported && has_flags) {
      return index;
    }
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed,
      "no Vulkan memory type satisfies the required flags"));
}

} // namespace cgpui
