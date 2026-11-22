#include "vulkan_composition_presenter_internal.hpp"

#if defined(_WIN32)
#include "../win32_composition/win32_composition_presenter_internal.hpp"

#include <optional>

namespace cgpui {
namespace {

std::optional<LUID> device_luid(VkPhysicalDevice device) {
  VkPhysicalDeviceIDProperties id{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
  VkPhysicalDeviceProperties2 properties{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &id};
  vkGetPhysicalDeviceProperties2(device, &properties);
  if (id.deviceLUIDValid != VK_TRUE) return std::nullopt;
  LUID luid{};
  static_assert(sizeof(luid) == VK_LUID_SIZE);
  std::memcpy(&luid, id.deviceLUID, sizeof(luid));
  return luid;
}

} // namespace

std::span<const char* const> vulkan_composition_device_extensions() {
  static constexpr std::array<const char*, 2> extensions{
      VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME,
      VK_KHR_WIN32_KEYED_MUTEX_EXTENSION_NAME,
  };
  return extensions;
}

bool vulkan_composition_device_suitable(VkPhysicalDevice device) {
  if (!device_luid(device)) return false;
  const VkPhysicalDeviceExternalImageFormatInfo external{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_IMAGE_FORMAT_INFO,
      .handleType = vulkan_composition_handle_type};
  const VkPhysicalDeviceImageFormatInfo2 format{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2,
      .pNext = &external,
      .format = vulkan_composition_format,
      .type = VK_IMAGE_TYPE_2D,
      .tiling = VK_IMAGE_TILING_OPTIMAL,
      .usage = vulkan_composition_usage};
  VkExternalImageFormatProperties external_properties{
      .sType = VK_STRUCTURE_TYPE_EXTERNAL_IMAGE_FORMAT_PROPERTIES};
  VkImageFormatProperties2 properties{
      .sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_PROPERTIES_2, .pNext = &external_properties};
  return vkGetPhysicalDeviceImageFormatProperties2(device, &format, &properties) == VK_SUCCESS &&
         (external_properties.externalMemoryProperties.externalMemoryFeatures &
          VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT) != 0;
}

VulkanCompositionPresenter::VulkanCompositionPresenter(
    Device device, std::shared_ptr<Win32CompositionPresenter> composition)
    : device_(device), composition_(std::move(composition)) {}

VulkanCompositionPresenter::~VulkanCompositionPresenter() { release_texture(); }

Result<std::unique_ptr<VulkanCompositionPresenter>> VulkanCompositionPresenter::create(
    Device device, const Win32SurfaceHandle& surface, VkExtent2D extent) {
  const std::optional<LUID> luid = device_luid(device.physical_device);
  if (!luid) {
    return std::unexpected(vulkan_error(
        ErrorCode::renderer_initialization_failed,
        "the Vulkan device reports no LUID for DirectComposition"));
  }
  auto composition = Win32CompositionPresenter::create(
      *luid, static_cast<HWND>(surface.hwnd),
      {.width = extent.width, .height = extent.height});
  if (!composition) return std::unexpected(composition.error());
  std::unique_ptr<VulkanCompositionPresenter> presenter(new VulkanCompositionPresenter(
      device, std::shared_ptr<Win32CompositionPresenter>(std::move(*composition))));
  if (auto result = presenter->import_texture(); !result) {
    return std::unexpected(result.error());
  }
  return presenter;
}

Result<void> VulkanCompositionPresenter::resize(VkExtent2D extent) {
  release_texture();
  if (auto result = composition_->resize({.width = extent.width, .height = extent.height});
      !result) {
    return result;
  }
  return import_texture();
}

} // namespace cgpui
#endif
