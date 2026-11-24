#include "vulkan_state_internal.hpp"

#include <limits>

namespace cgpui {

Result<void> VulkanRendererState::require_live_device() const {
  if (device_recovery_error_) return std::unexpected(*device_recovery_error_);
  if (device_recovery_terminal_ || device_ == VK_NULL_HANDLE) {
    return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
        "Vulkan device is unavailable"));
  }
  return {};
}

Result<void> VulkanRendererState::recover_after_device_loss(
    VkResult failure, std::string message) {
  // Native evidence is mandatory: OOM/invalid commands/surface statuses cannot
  // authorize destructive device recovery, nor can flattened message text.
  if (failure != VK_ERROR_DEVICE_LOST) return require_vk_success(failure, std::move(message));
  if (device_recovery_terminal_) return require_live_device();
  device_recovery_error_ = vulkan_error(ErrorCode::renderer_initialization_failed,
      std::move(message) + "; Vulkan device recovery did not complete");
  device_recovery_terminal_ = true;
  presentation_blocked_ = true;
  image_restore_pending_ = true;
  const VkPhysicalDevice preferred = physical_device_;
  destroy_device_resources(false);
  if (device_generation_ == std::numeric_limits<std::uint64_t>::max()) {
    device_recovery_error_->message += "; device generation exhausted";
    return require_live_device();
  }
  try {
    auto recreated = select_physical_device(preferred);
    if (recreated) recreated = create_device_resources();
    if (!recreated) {
      destroy_device_resources(false);
      device_recovery_error_->message += "; " + recreated.error().message;
      return require_live_device();
    }
  } catch (...) {
    // Allocation/host exceptions must not leave a partially created GPU cohort.
    // The terminal guard remains set; callers cannot repeat the failed rebuild.
    destroy_device_resources(false);
    throw;
  }
  ++device_generation_;
  device_recovery_error_.reset();
  device_recovery_terminal_ = false;
  presentation_blocked_ = false;
  return std::unexpected(vulkan_error(ErrorCode::frame_retry_required,
      "Vulkan device was recreated; the lost frame was not presented"));
}

} // namespace cgpui
