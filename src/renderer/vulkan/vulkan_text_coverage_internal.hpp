#pragma once

#include "cgpui/renderer/renderer_types.hpp"

namespace cgpui {

enum class VulkanTextCoverageTransfer {
  linear,
  power,
};

enum class VulkanTextAlphaMode {
  straight_color_coverage_alpha,
};

struct VulkanTextCoveragePolicy {
  VulkanTextCoverageTransfer transfer = VulkanTextCoverageTransfer::linear;
  float gamma = 1.0F;
  VulkanTextAlphaMode alpha_mode =
      VulkanTextAlphaMode::straight_color_coverage_alpha;
};

[[nodiscard]] VulkanTextCoveragePolicy
vulkan_default_text_coverage_policy();
[[nodiscard]] float vulkan_resolve_text_coverage(
    float sampled_coverage,
    VulkanTextCoveragePolicy policy);
[[nodiscard]] Color vulkan_apply_text_coverage(
    Color color,
    float sampled_coverage,
    VulkanTextCoveragePolicy policy);

} // namespace cgpui
