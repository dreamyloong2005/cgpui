#include "vulkan_text_coverage_internal.hpp"

#include <algorithm>
#include <cmath>

namespace cgpui {

VulkanTextCoveragePolicy vulkan_default_text_coverage_policy() {
  return {};
}

float vulkan_resolve_text_coverage(
    float sampled_coverage,
    VulkanTextCoveragePolicy policy) {
  const float coverage = std::clamp(sampled_coverage, 0.0F, 1.0F);
  if (policy.transfer != VulkanTextCoverageTransfer::power) {
    return coverage;
  }
  const float gamma =
      std::isfinite(policy.gamma) && policy.gamma > 0.0F
          ? policy.gamma
          : 1.0F;
  return std::pow(coverage, gamma);
}

Color vulkan_apply_text_coverage(
    Color color,
    float sampled_coverage,
    VulkanTextCoveragePolicy policy) {
  const float coverage =
      vulkan_resolve_text_coverage(sampled_coverage, policy);
  color.a *= coverage;
  return color;
}

} // namespace cgpui
