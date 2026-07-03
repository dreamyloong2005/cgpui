#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::recover_after_failed_submit(
    std::string message) {
  presentation_blocked_ = true;
  (void)vkDeviceWaitIdle(device_);
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed, std::move(message)));
}

Result<void> VulkanRendererState::recover_after_failed_acquire(
    ErrorCode code,
    std::string message,
    bool block_presentation) {
  presentation_blocked_ = block_presentation;
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(code, std::move(message)));
}

Result<void> VulkanRendererState::recover_after_failed_present(
    std::string message) {
  presentation_blocked_ = true;
  (void)vkDeviceWaitIdle(device_);
  if (auto result = create_sync_objects(); !result) {
    return std::unexpected(result.error());
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed, std::move(message)));
}

} // namespace cgpui
