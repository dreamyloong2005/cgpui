#include "vulkan_internal.hpp"

namespace cgpui {

Result<void> VulkanRendererState::create_instance() {
#if defined(_WIN32) || defined(__linux__)
#if defined(_WIN32)
  const std::array<const char*, 2> extensions{
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
  };
#else
  const std::array<const char*, 2> extensions{
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
  };
#endif

  const VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "CGPUI",
      .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
      .pEngineName = "CGPUI",
      .engineVersion = VK_MAKE_VERSION(0, 1, 0),
      .apiVersion = VK_API_VERSION_1_0,
  };
  const VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
  };

  return require_vk_success(
      vkCreateInstance(&create_info, nullptr, &instance_),
      "vkCreateInstance failed");
#else
  return std::unexpected(vulkan_error(
      ErrorCode::unsupported_platform,
      "Vulkan renderer Win32 surface initialization requires Windows"));
#endif
}

Result<void> VulkanRendererState::create_surface() {
#if defined(_WIN32)
  auto surface = require_win32_surface(descriptor_.native_surface);
  if (!surface) {
    return std::unexpected(surface.error());
  }
  const VkWin32SurfaceCreateInfoKHR create_info{
      .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
      .hinstance = static_cast<HINSTANCE>(surface->hinstance),
      .hwnd = static_cast<HWND>(surface->hwnd),
  };

  return require_vk_success(
      vkCreateWin32SurfaceKHR(instance_, &create_info, nullptr, &surface_),
      "vkCreateWin32SurfaceKHR failed");
#elif defined(__linux__)
  auto surface = require_wayland_surface(descriptor_.native_surface);
  if (!surface) {
    return std::unexpected(surface.error());
  }
  const VkWaylandSurfaceCreateInfoKHR create_info{
      .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
      .display = static_cast<wl_display*>(surface->display),
      .surface = static_cast<wl_surface*>(surface->surface),
  };

  return require_vk_success(
      vkCreateWaylandSurfaceKHR(instance_, &create_info, nullptr, &surface_),
      "vkCreateWaylandSurfaceKHR failed");
#else
  return std::unexpected(vulkan_error(
      ErrorCode::unsupported_platform,
      "Vulkan renderer surface initialization requires Windows or Linux"));
#endif
}

} // namespace cgpui
