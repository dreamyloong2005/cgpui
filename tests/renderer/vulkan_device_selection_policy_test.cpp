#include "vulkan_device_selection_internal.hpp"
#include <array>
#include <limits>

using namespace cgpui;
int main() {
  const auto automatic = vulkan_parse_device_selection("");
  if (!automatic || automatic->has_value()) return 1;
  for (const auto& [text, expected] : std::array<std::pair<const char*, std::uint32_t>, 4>{
      {{"0", 0}, {"12", 12}, {"0001", 1}, {"4294967295", std::numeric_limits<std::uint32_t>::max()}}}) {
    const auto parsed = vulkan_parse_device_selection(text);
    if (!parsed || *parsed != std::optional{expected}) return 2;
  }
  for (const char* text : {" ", "auto", "warp", "-1", "+1", " 1", "1 ", "1x", "1.0", "4294967296", "999999999999999999999"}) {
    const auto parsed = vulkan_parse_device_selection(text);
    if (parsed || parsed.error().code != ErrorCode::invalid_argument) return 3;
  }
  // Opaque identities only, never sent to a native API or dereferenced.
  const auto a = reinterpret_cast<VkPhysicalDevice>(std::uintptr_t{1});
  const auto b = reinterpret_cast<VkPhysicalDevice>(std::uintptr_t{2});
  const auto c = reinterpret_cast<VkPhysicalDevice>(std::uintptr_t{3});
  std::vector<VkPhysicalDevice> devices{a, b, c};
  if (!vulkan_filter_device_candidates(devices, std::nullopt, b) || devices != std::vector{a, b, c}) return 4;
  if (!vulkan_filter_device_candidates(devices, 1, VK_NULL_HANDLE) || devices != std::vector{b}) return 5;
  devices = {c, a, b};
  if (!vulkan_filter_device_candidates(devices, 1, b) || devices != std::vector{b}) return 6;
  devices = {a, c};
  const auto unavailable = vulkan_filter_device_candidates(devices, 1, b);
  if (unavailable || unavailable.error().code != ErrorCode::renderer_initialization_failed ||
      devices != std::vector{a, c}) return 7;
  const auto outside = vulkan_filter_device_candidates(devices, 2, VK_NULL_HANDLE);
  if (outside || devices != std::vector{a, c}) return 8;
  devices.clear();
  if (vulkan_filter_device_candidates(devices, 0, VK_NULL_HANDLE) ||
      !vulkan_filter_device_candidates(devices, std::nullopt, VK_NULL_HANDLE) || !devices.empty()) return 9;
  return 0;
}
