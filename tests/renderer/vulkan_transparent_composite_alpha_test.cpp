#include "vulkan_platform_internal.hpp"

int main() {
  constexpr VkCompositeAlphaFlagsKHR opaque_and_pre =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
  if (cgpui::choose_vulkan_composite_alpha(opaque_and_pre, false) !=
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
    return 1;
  }
  if (cgpui::choose_vulkan_composite_alpha(opaque_and_pre, true) !=
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
    return 2;
  }
  constexpr VkCompositeAlphaFlagsKHR opaque_and_post =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
  if (cgpui::choose_vulkan_composite_alpha(opaque_and_post, true) !=
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
    return 3;
  }
  constexpr VkCompositeAlphaFlagsKHR opaque_and_inherit =
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR |
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
  if (cgpui::choose_vulkan_composite_alpha(opaque_and_inherit, true) !=
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
    return 4;
  }
  return cgpui::choose_vulkan_composite_alpha(
             VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, true) ==
          VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
      ? 0
      : 5;
}
