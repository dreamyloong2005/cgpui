#pragma once
#include "vulkan_loader_windows_internal.hpp"

namespace cgpui_test {
struct VulkanImageCreateFailureObservation {
  unsigned create_attempts = 0, images_destroyed_before_failure = 0;
  bool injected = false, safely_quiesced = false;
};
// One-shot native CreateImage OOM, preceded by a real successful idle wait.
class VulkanImageCreateFailureHook {
 public:
  VulkanImageCreateFailureHook();
  ~VulkanImageCreateFailureHook();
  VulkanImageCreateFailureHook(const VulkanImageCreateFailureHook&) = delete;
  VulkanImageCreateFailureHook& operator=(const VulkanImageCreateFailureHook&) = delete;
  bool installed() const;
  VulkanImageCreateFailureObservation observation() const;
 private:
  bool installed_ = false;
};
} // namespace cgpui_test
