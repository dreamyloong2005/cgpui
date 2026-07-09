#pragma once

namespace cgpui {

enum class VulkanRoundedRectAntialiasingMode {
  coverage_fringe,
};

struct VulkanRoundedRectAntialiasingPolicy {
  VulkanRoundedRectAntialiasingMode mode =
      VulkanRoundedRectAntialiasingMode::coverage_fringe;
  float fringe_width = 1.0F;
};

[[nodiscard]] VulkanRoundedRectAntialiasingPolicy
vulkan_default_rounded_rect_antialiasing_policy();
[[nodiscard]] float vulkan_rounded_rect_coverage(float coverage);

} // namespace cgpui
