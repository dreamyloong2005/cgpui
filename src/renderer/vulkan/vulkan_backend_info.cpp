#include "vulkan_state_internal.hpp"

#include <cstring>

namespace cgpui {

RendererBackendInfo VulkanRendererState::backend_info() const {
  if (physical_device_ == VK_NULL_HANDLE) return {};
  VkPhysicalDeviceIDProperties identity{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
  VkPhysicalDeviceProperties2 properties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
      .pNext = &identity};
  vkGetPhysicalDeviceProperties2(physical_device_, &properties);
  RendererBackendInfo info{
      .backend = RendererBackend::vulkan,
      .adapter_name = properties.properties.deviceName,
      .vendor_id = properties.properties.vendorID,
      .software_adapter = properties.properties.deviceType ==
                          VK_PHYSICAL_DEVICE_TYPE_CPU,
      .selection = RendererBackendSelection::vulkan,
      // Swapchain recreation does not replace the underlying VkDevice.
      .device_generation = device_ != VK_NULL_HANDLE ? 1U : 0U,
  };
  if (identity.deviceLUIDValid == VK_TRUE) {
    std::uint64_t luid = 0;
    static_assert(sizeof(luid) == VK_LUID_SIZE);
    std::memcpy(&luid, identity.deviceLUID, sizeof(luid));
    info.adapter_luid = luid;
  }
  return info;
}

} // namespace cgpui
