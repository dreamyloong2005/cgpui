#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::allocate_swapchain_command_buffers(
    VulkanSwapchainResources& resources) {
  resources.command_buffers.resize(resources.images.size());
  const VkCommandBufferAllocateInfo allocate_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<std::uint32_t>(resources.command_buffers.size()),
  };
  if (auto result = require_vk_success(
          vkAllocateCommandBuffers(
              device_, &allocate_info, resources.command_buffers.data()),
          "vkAllocateCommandBuffers failed");
      !result) {
    resources.command_buffers.clear();
    return std::unexpected(result.error());
  }
  return {};
}

} // namespace cgpui
