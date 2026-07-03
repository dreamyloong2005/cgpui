#include "vulkan_internal.hpp"

namespace cgpui {

Error vulkan_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

Result<void> require_vk_success(VkResult result, std::string message) {
  if (result != VK_SUCCESS) {
    message += " (VkResult ";
    message += std::to_string(static_cast<int>(result));
    message += ")";
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        std::move(message)));
  }

  return {};
}

Result<Win32SurfaceHandle> require_win32_surface(
    const NativeSurfaceHandle& native_surface) {
  const auto* surface = std::get_if<Win32SurfaceHandle>(&native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a Win32 native surface",
    });
  }

  if (surface->hinstance == nullptr || surface->hwnd == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires non-null Win32 handles",
    });
  }

  return *surface;
}

Result<WaylandSurfaceHandle> require_wayland_surface(
    const NativeSurfaceHandle& native_surface) {
  const auto* surface = std::get_if<WaylandSurfaceHandle>(&native_surface);
  if (surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires a Wayland native surface",
    });
  }

  if (surface->display == nullptr || surface->surface == nullptr) {
    return std::unexpected(Error{
        .code = ErrorCode::renderer_initialization_failed,
        .message = "Vulkan renderer requires non-null Wayland handles",
    });
  }

  return *surface;
}

} // namespace cgpui
