#pragma once

#include "vulkan_platform_internal.hpp"

#include <array>
#include <memory>
#include <span>

namespace cgpui {

class Win32CompositionPresenter;

// Composition surfaces (Win32SurfaceHandle::composition; Win32 popup
// windows set it): the renderer draws into one image imported from the
// shared D3D11 texture of a Win32CompositionPresenter
// (src/renderer/win32_composition) and presents through DirectComposition
// instead of a VkSwapchainKHR.
[[nodiscard]] bool vulkan_composition_surface(const NativeSurfaceHandle& surface);
// VK_KHR_external_memory_win32 and VK_KHR_win32_keyed_mutex; none off Windows.
[[nodiscard]] std::span<const char* const> vulkan_composition_device_extensions();
// The device reports its LUID and imports a B8G8R8A8_UNORM D3D11 texture
// as a color attachment and copy source.
[[nodiscard]] bool vulkan_composition_device_suitable(VkPhysicalDevice device);
// The layout a frame leaves the composition image in; the release to
// D3D11 starts from it (PRESENT_SRC_KHR is for presentable images only).
inline constexpr VkImageLayout vulkan_composition_frame_layout =
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
// The imported texture: the module's B8G8R8A8_UNORM D3D11 texture, drawn
// into and copied from (pixel capture).
inline constexpr VkFormat vulkan_composition_format = VK_FORMAT_B8G8R8A8_UNORM;
inline constexpr VkImageUsageFlags vulkan_composition_usage =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
inline constexpr VkExternalMemoryHandleTypeFlagBits vulkan_composition_handle_type =
    VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT;

class VulkanCompositionPresenter {
 public:
  struct Device {
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    std::uint32_t queue_family = 0;
  };

  // The D3D11 side on the adapter of `device` (its LUID), and the import
  // of its texture.
  static Result<std::unique_ptr<VulkanCompositionPresenter>> create(
      Device device, const Win32SurfaceHandle& surface, VkExtent2D extent);
  ~VulkanCompositionPresenter();
  VulkanCompositionPresenter(const VulkanCompositionPresenter&) = delete;
  VulkanCompositionPresenter& operator=(const VulkanCompositionPresenter&) = delete;

  // A new texture of `extent`, imported again. Nothing may still use the
  // previous image (the device is idle and its views are destroyed).
  Result<void> resize(VkExtent2D extent);
  [[nodiscard]] VkImage image() const { return image_; }
  [[nodiscard]] VkExtent2D extent() const { return extent_; }
  // Submits `frame` between the transfers from and to
  // VK_QUEUE_FAMILY_EXTERNAL, acquiring the keyed mutex with the render
  // key and releasing it with the present key; signals `fence`.
  Result<void> submit(VkQueue queue, VkCommandBuffer frame, VkFence fence) const;
  // The D3D11 side copies the submitted frame into the composition
  // swapchain and presents it.
  Result<void> present();

 private:
  VulkanCompositionPresenter(
      Device device, std::shared_ptr<Win32CompositionPresenter> composition);
  Result<void> import_texture();
  Result<void> record_transfers();
  void release_texture();

  Device device_;
  std::shared_ptr<Win32CompositionPresenter> composition_;
  VkImage image_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  // Recorded once per import: [0] acquires the image from
  // VK_QUEUE_FAMILY_EXTERNAL, [1] releases it to D3D11.
  std::array<VkCommandBuffer, 2> transfers_{};
  VkExtent2D extent_{};
};

} // namespace cgpui
