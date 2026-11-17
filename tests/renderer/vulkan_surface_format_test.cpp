#include "vulkan_platform_internal.hpp"

#include <array>
#include <iostream>

namespace {

int fail(int code, const char* message) {
  std::cerr << "vulkan surface format: " << message << '\n';
  return code;
}

constexpr VkSurfaceFormatKHR format(VkFormat value) {
  return VkSurfaceFormatKHR{
      .format = value, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
}

} // namespace

// Color components are sRGB-encoded and blended in that space, so the
// swapchain must store them unchanged (UNORM). An *_SRGB format would encode
// them a second time: rgb(32,34,38) was shown as rgb(99,102,108).
int main() {
  // The NVIDIA and Intel win32 surfaces on the reference machine list SRGB
  // before UNORM.
  const std::array listed{
      format(VK_FORMAT_B8G8R8A8_SRGB), format(VK_FORMAT_B8G8R8A8_UNORM),
      format(VK_FORMAT_R8G8B8A8_SRGB), format(VK_FORMAT_R8G8B8A8_UNORM)};
  if (cgpui::choose_vulkan_surface_format(listed).format !=
      VK_FORMAT_B8G8R8A8_UNORM) {
    return fail(1, "B8G8R8A8_UNORM must win over SRGB formats");
  }
  const std::array rgba_only{
      format(VK_FORMAT_R8G8B8A8_SRGB), format(VK_FORMAT_R8G8B8A8_UNORM)};
  if (cgpui::choose_vulkan_surface_format(rgba_only).format !=
      VK_FORMAT_R8G8B8A8_UNORM) {
    return fail(2, "R8G8B8A8_UNORM must be used when BGRA UNORM is absent");
  }
  VkSurfaceFormatKHR wrong_space = format(VK_FORMAT_B8G8R8A8_UNORM);
  wrong_space.colorSpace = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT;
  const std::array other_space{wrong_space, format(VK_FORMAT_A2B10G10R10_UNORM_PACK32)};
  if (cgpui::choose_vulkan_surface_format(other_space).format !=
      VK_FORMAT_B8G8R8A8_UNORM) {
    return fail(3, "without an sRGB-nonlinear UNORM match keep the first format");
  }
  return 0;
}
