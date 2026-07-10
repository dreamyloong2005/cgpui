#include "vulkan_internal.hpp"

namespace cgpui {

VkSurfaceFormatKHR choose_vulkan_surface_format(
    std::span<const VkSurfaceFormatKHR> formats) {
  const auto preferred = std::ranges::find_if(
      formats, [](const VkSurfaceFormatKHR& format) {
        return format.format == VK_FORMAT_B8G8R8A8_SRGB &&
               format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      });
  return preferred != formats.end() ? *preferred : formats.front();
}

VkCompositeAlphaFlagBitsKHR choose_vulkan_composite_alpha(
    VkCompositeAlphaFlagsKHR supported_alpha,
    bool transparent_background) {
  constexpr std::array opaque_preference{
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  constexpr std::array transparent_preference{
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
  };
  const auto& preference =
      transparent_background ? transparent_preference : opaque_preference;
  for (VkCompositeAlphaFlagBitsKHR alpha : preference) {
    if ((supported_alpha & alpha) != 0) {
      return alpha;
    }
  }

  return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

VkExtent2D choose_vulkan_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    Size framebuffer_size) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  return VkExtent2D{
      .width = std::clamp(
          static_cast<std::uint32_t>(framebuffer_size.width),
          capabilities.minImageExtent.width,
          capabilities.maxImageExtent.width),
      .height = std::clamp(
          static_cast<std::uint32_t>(framebuffer_size.height),
          capabilities.minImageExtent.height,
          capabilities.maxImageExtent.height),
  };
}

} // namespace cgpui
