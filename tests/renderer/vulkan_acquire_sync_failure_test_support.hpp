#pragma once
#include "vulkan_loader_windows_internal.hpp"

namespace cgpui_test {
struct VulkanAcquireSyncObservation {
  unsigned idle_successes = 0, semaphore_destroys = 0, semaphore_attempts = 0, fence_destroys = 0;
  unsigned fence_waits = 0, null_fence_waits = 0;
  bool injected = false, teardown_before_idle = false, create_before_idle = false;
};
// Installed after a successful capture and used with the safely drained acquire
// fault from VulkanRecoveryHooks. Observes native ordering, never renderer state.
class VulkanAcquireSyncFailureHook {
 public:
  VulkanAcquireSyncFailureHook();
  ~VulkanAcquireSyncFailureHook();
  VulkanAcquireSyncFailureHook(const VulkanAcquireSyncFailureHook&) = delete;
  VulkanAcquireSyncFailureHook& operator=(const VulkanAcquireSyncFailureHook&) = delete;
  bool installed() const;
  VulkanAcquireSyncObservation observation() const;
 private:
  bool installed_ = false;
};
} // namespace cgpui_test
