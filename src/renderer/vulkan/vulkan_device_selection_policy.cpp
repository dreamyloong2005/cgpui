#include "vulkan_device_selection_internal.hpp"

#include <charconv>

namespace cgpui {
Result<std::optional<std::uint32_t>> vulkan_parse_device_selection(std::string_view value) {
  if (value.empty()) return std::optional<std::uint32_t>{};
  std::uint32_t index = 0;
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), index);
  if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size())
    return std::unexpected(vulkan_error(ErrorCode::invalid_argument,
        "CGPUI_VULKAN_DEVICE must be an unsigned decimal adapter index"));
  return std::optional{index};
}
Result<void> vulkan_filter_device_candidates(std::vector<VkPhysicalDevice>& devices,
    std::optional<std::uint32_t> requested_index, VkPhysicalDevice preferred) {
  if (!requested_index) return {}; // Existing automatic suitability/alpha preference remains the owner.
  VkPhysicalDevice selected = VK_NULL_HANDLE;
  if (preferred != VK_NULL_HANDLE) {
    // Recovery uses the selected identity, never a possibly reordered index or
    // a new environment value. Explicit selection cannot silently switch GPUs.
    if (std::ranges::find(devices, preferred) == devices.end())
      return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
          "The explicitly selected Vulkan device is no longer available"));
    selected = preferred;
  } else {
    if (*requested_index >= devices.size())
      return std::unexpected(vulkan_error(ErrorCode::renderer_initialization_failed,
          "CGPUI_VULKAN_DEVICE index is outside the enumerated adapter range"));
    selected = devices[*requested_index];
  }
  devices.resize(1); // Shrink only; candidate filtering never allocates.
  devices.front() = selected;
  return {};
}
} // namespace cgpui
