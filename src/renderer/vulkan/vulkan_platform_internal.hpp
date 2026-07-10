#pragma once

#include "cgpui/renderer/renderer.hpp"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#else
#include <vulkan/vulkan.h>
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <expected>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cgpui {

Error vulkan_error(ErrorCode code, std::string message);
Result<void> require_vk_success(VkResult result, std::string message);
Result<Win32SurfaceHandle> require_win32_surface(
    const NativeSurfaceHandle& native_surface);
Result<WaylandSurfaceHandle> require_wayland_surface(
    const NativeSurfaceHandle& native_surface);

[[nodiscard]] VkSurfaceFormatKHR choose_vulkan_surface_format(
    std::span<const VkSurfaceFormatKHR> formats);
[[nodiscard]] VkCompositeAlphaFlagBitsKHR choose_vulkan_composite_alpha(
    VkCompositeAlphaFlagsKHR supported_alpha,
    bool transparent_background);
[[nodiscard]] VkExtent2D choose_vulkan_extent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    Size framebuffer_size);
} // namespace cgpui
