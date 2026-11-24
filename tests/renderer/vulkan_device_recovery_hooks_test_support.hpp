#pragma once
#include "vulkan_loader_windows_internal.hpp"
#include <cstdint>
#include <optional>

namespace cgpui_test {
enum class VulkanLossSite { none, acquire, submit, present, pacing_wait, capture_wait, resize_wait };
enum class VulkanRebuildFailure { none, device, swapchain };
struct VulkanRecoveryObservation {
  bool reliable = true, injected = false, safely_quiesced = false;
  unsigned device_attempts = 0, devices_created = 0, devices_destroyed = 0, live_devices = 0;
  unsigned instances_created = 0, instances_destroyed = 0, surfaces_created = 0, surfaces_destroyed = 0;
  unsigned rebuild_failures = 0;
  std::optional<std::uint32_t> initial_enumeration_index;
  VkPhysicalDevice initial_physical = VK_NULL_HANDLE, latest_physical = VK_NULL_HANDLE;
};
// Serial, process-local test simulation, NOT actual driver device removal.
// Every injected failure first drains the real healthy device with DeviceWaitIdle.
// Only imported native calls are observed/replaced; production state is never mutated.
class VulkanRecoveryHooks {
 public:
  VulkanRecoveryHooks();
  ~VulkanRecoveryHooks();
  VulkanRecoveryHooks(const VulkanRecoveryHooks&) = delete;
  VulkanRecoveryHooks& operator=(const VulkanRecoveryHooks&) = delete;
  [[nodiscard]] bool installed() const { return installed_; }
  void arm(VulkanLossSite site, VkResult result = VK_ERROR_DEVICE_LOST,
      VulkanRebuildFailure rebuild_failure = VulkanRebuildFailure::none);
  [[nodiscard]] VulkanRecoveryObservation observation() const;
 private:
  bool installed_ = false;
};
} // namespace cgpui_test
