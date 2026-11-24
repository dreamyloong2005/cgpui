#pragma once
#include <array>
#include <cstdint>

namespace cgpui_test {
enum class VulkanObservedDrawKind { indexed, direct };
struct VulkanObservedDraw {
  VulkanObservedDrawKind kind = VulkanObservedDrawKind::indexed;
  std::uint32_t count = 0, instances = 0, first = 0;
  std::int32_t vertex_offset = 0;
  std::uint32_t first_instance = 0;
  friend bool operator==(const VulkanObservedDraw&, const VulkanObservedDraw&) = default;
};
struct VulkanObservedDrawSequence {
  std::array<VulkanObservedDraw, 4> draws{};
  unsigned size = 0;
  bool overflow = false;
};
void append_vulkan_observed_draw(VulkanObservedDrawSequence& sequence, VulkanObservedDraw draw) noexcept;
bool vulkan_observed_missing_scene_matches(const VulkanObservedDrawSequence& sequence, unsigned available_mask) noexcept;
} // namespace cgpui_test
