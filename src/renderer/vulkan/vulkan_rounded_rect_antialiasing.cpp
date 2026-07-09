#include "vulkan_rounded_rect_antialiasing_internal.hpp"

#include <algorithm>

namespace cgpui {

VulkanRoundedRectAntialiasingPolicy
vulkan_default_rounded_rect_antialiasing_policy() {
  return {};
}

float vulkan_rounded_rect_coverage(float coverage) {
  return std::clamp(coverage, 0.0F, 1.0F);
}

} // namespace cgpui
