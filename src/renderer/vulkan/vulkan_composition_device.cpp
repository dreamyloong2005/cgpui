#include "vulkan_internal.hpp"
#include "vulkan_device_selection_internal.hpp"

#include <optional>

namespace cgpui {
namespace {

std::optional<std::uint32_t> graphics_queue_family(VkPhysicalDevice device) {
  std::uint32_t count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
  std::vector<VkQueueFamilyProperties> families(count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
  for (std::uint32_t index = 0; index < count; ++index) {
    if (families[index].queueCount > 0 &&
        (families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      return index;
    }
  }
  return std::nullopt;
}

} // namespace

bool vulkan_composition_surface(const NativeSurfaceHandle& surface) {
  const auto* win32 = std::get_if<Win32SurfaceHandle>(&surface);
  return win32 != nullptr && win32->composition;
}

bool VulkanRendererState::composition_surface() const {
  return vulkan_composition_surface(descriptor_.native_surface);
}

// No surface to present to: the first Vulkan 1.1 device with graphics that
// imports D3D11 textures with their keyed mutex. Its queue renders and
// submits; D3D11 on the same adapter presents.
Result<void> VulkanRendererState::select_composition_device(VkPhysicalDevice preferred) {
  std::uint32_t device_count = 0;
  if (auto result = require_vk_success(
          vkEnumeratePhysicalDevices(instance_, &device_count, nullptr),
          "vkEnumeratePhysicalDevices failed");
      !result) {
    return result;
  }
  std::vector<VkPhysicalDevice> devices(device_count);
  if (auto result = require_vk_success(
          vkEnumeratePhysicalDevices(instance_, &device_count, devices.data()),
          "vkEnumeratePhysicalDevices failed");
      !result) {
    return result;
  }
  devices.resize(device_count);
  if (auto selected = vulkan_filter_device_candidates(devices, requested_device_index_, preferred); !selected)
    return selected;
  if (const auto previous = std::ranges::find(devices, preferred); previous != devices.end()) {
    std::rotate(devices.begin(), previous, previous + 1);
  }
  const std::span<const char* const> extensions = vulkan_composition_device_extensions();
  for (VkPhysicalDevice device : devices) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(device, &properties);
    const std::optional<std::uint32_t> family = graphics_queue_family(device);
    if (properties.apiVersion < VK_API_VERSION_1_1 || !family || extensions.empty()) {
      continue;
    }
    bool supported = true;
    for (const char* extension : extensions) {
      auto has = has_device_extension(device, extension);
      if (!has) return std::unexpected(has.error());
      supported = supported && *has;
    }
    if (!supported || !vulkan_composition_device_suitable(device)) continue;
    physical_device_ = device;
    graphics_queue_family_ = *family;
    present_queue_family_ = *family;
    return {};
  }
  return std::unexpected(vulkan_error(
      ErrorCode::renderer_initialization_failed,
      "No Vulkan 1.1 device with graphics imports D3D11 textures "
      "(VK_KHR_external_memory_win32, VK_KHR_win32_keyed_mutex) for "
      "DirectComposition presentation"));
}

} // namespace cgpui
