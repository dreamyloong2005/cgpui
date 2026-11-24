#pragma once
#include "vulkan_loader_windows_internal.hpp"
#include <cstdint>

namespace cgpui_test {
// Installed only after a real reconstruction completes; no renderer state edits.
class VulkanImageLimitHook {
 public:
  VulkanImageLimitHook(VkPhysicalDevice physical, std::uint32_t maximum);
  ~VulkanImageLimitHook();
  VulkanImageLimitHook(const VulkanImageLimitHook&) = delete;
  VulkanImageLimitHook& operator=(const VulkanImageLimitHook&) = delete;
  bool installed() const;
  void restore_properties();
  unsigned property_overrides() const;
  unsigned image_attempts() const;
  unsigned invalid_image_attempts() const;
 private:
  bool installed_ = false;
};
} // namespace cgpui_test
