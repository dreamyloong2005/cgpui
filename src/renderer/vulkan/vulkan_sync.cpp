#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::create_command_pool() {
  const VkCommandPoolCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = graphics_queue_family_,
  };
  return require_vk_success(
      vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_),
      "vkCreateCommandPool failed");
}

void VulkanRendererState::destroy_sync_objects() {
  if (in_flight_ != VK_NULL_HANDLE) {
    vkDestroyFence(device_, in_flight_, nullptr);
    in_flight_ = VK_NULL_HANDLE;
  }
  if (render_finished_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(device_, render_finished_, nullptr);
    render_finished_ = VK_NULL_HANDLE;
  }
  if (image_available_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(device_, image_available_, nullptr);
    image_available_ = VK_NULL_HANDLE;
  }
}

Result<void> VulkanRendererState::create_sync_objects() {
  destroy_sync_objects();

  const VkSemaphoreCreateInfo semaphore_info{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  const VkFenceCreateInfo fence_info{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = VK_FENCE_CREATE_SIGNALED_BIT,
  };
  if (auto result = require_vk_success(
          vkCreateSemaphore(device_, &semaphore_info, nullptr, &image_available_),
          "vkCreateSemaphore failed");
      !result) {
    destroy_sync_objects();
    return result;
  }
  if (auto result = require_vk_success(
          vkCreateSemaphore(device_, &semaphore_info, nullptr, &render_finished_),
          "vkCreateSemaphore failed");
      !result) {
    destroy_sync_objects();
    return result;
  }
  if (auto result = require_vk_success(
          vkCreateFence(device_, &fence_info, nullptr, &in_flight_),
          "vkCreateFence failed");
      !result) {
    destroy_sync_objects();
    return result;
  }

  return {};
}

} // namespace cgpui
