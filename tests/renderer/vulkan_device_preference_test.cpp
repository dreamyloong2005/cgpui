#include "vulkan_device_preference_internal.hpp"

#include <iostream>

namespace {

constexpr VkCompositeAlphaFlagsKHR opaque_inherit =
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
constexpr VkCompositeAlphaFlagsKHR opaque_premultiplied =
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;

cgpui::VulkanDeviceCandidate candidate(VkCompositeAlphaFlagsKHR alpha) {
  return cgpui::VulkanDeviceCandidate{.composite_alpha = alpha};
}

int fail(int code, const char* message) {
  std::cerr << "vulkan device preference: " << message << '\n';
  return code;
}

} // namespace

int main() {
  using cgpui::vulkan_prefer_device_candidate;
  const auto integrated = candidate(opaque_inherit);
  const auto discrete = candidate(opaque_premultiplied);
  const auto unknown = candidate(0);
  // Measured on this machine: Intel 0x9 first, NVIDIA 0x3 second.
  if (!vulkan_prefer_device_candidate(discrete, integrated, true)) {
    return fail(1, "transparent surface must prefer premultiplied alpha");
  }
  if (vulkan_prefer_device_candidate(discrete, integrated, false)) {
    return fail(2, "opaque surface must keep enumeration order");
  }
  if (vulkan_prefer_device_candidate(integrated, discrete, true)) {
    return fail(3, "a non-premultiplied device must not replace one");
  }
  if (vulkan_prefer_device_candidate(discrete, discrete, true)) {
    return fail(4, "equal candidates must keep the earlier device");
  }
  if (vulkan_prefer_device_candidate(unknown, integrated, true) ||
      !vulkan_prefer_device_candidate(discrete, unknown, true)) {
    return fail(5, "a failed capability query only loses the preference");
  }
  return 0;
}
