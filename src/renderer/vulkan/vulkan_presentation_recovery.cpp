#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::recover_after_failed_submit(
    std::string message) {
  vulkan_invalidate_frame_command_reuse(command_reuse_states_);
  presentation_blocked_ = true;
  if (const auto idle = vkDeviceWaitIdle(device_); idle != VK_SUCCESS)
    return recover_after_device_loss(idle, "Vulkan failed-submit retirement wait failed");
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed, std::move(message)));
}

Result<void> VulkanRendererState::recover_after_failed_record(
    std::string message) {
  vulkan_invalidate_frame_command_reuse(command_reuse_states_);
  presentation_blocked_ = true;
  if (const auto idle = vkDeviceWaitIdle(device_); idle != VK_SUCCESS)
    return recover_after_device_loss(idle, "Vulkan failed-record retirement wait failed");
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed,
      std::move(message)));
}

Result<void> VulkanRendererState::recover_after_failed_acquire(
    ErrorCode code,
    std::string message,
    bool block_presentation) {
  vulkan_invalidate_frame_command_reuse(command_reuse_states_);
  presentation_blocked_ = block_presentation;
  // The submit fence does not retire a later present-queue semaphore wait.
  if (const auto idle = vkDeviceWaitIdle(device_); idle != VK_SUCCESS) {
    presentation_blocked_ = true;
    return recover_after_device_loss(idle, "Vulkan failed-acquire retirement wait failed");
  }
  if (auto result = create_sync_objects(); !result) {
    presentation_blocked_ = true;
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(code, std::move(message)));
}

Result<void> VulkanRendererState::recover_after_failed_present(
    std::string message) {
  vulkan_invalidate_frame_command_reuse(command_reuse_states_);
  presentation_blocked_ = true;
  if (const auto idle = vkDeviceWaitIdle(device_); idle != VK_SUCCESS)
    return recover_after_device_loss(idle, "Vulkan failed-present retirement wait failed");
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed, std::move(message)));
}

} // namespace cgpui
