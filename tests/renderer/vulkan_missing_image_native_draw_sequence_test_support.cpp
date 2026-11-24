#include "vulkan_missing_image_native_draw_sequence_test_support.hpp"

namespace cgpui_test {
void append_vulkan_observed_draw(VulkanObservedDrawSequence& sequence, VulkanObservedDraw draw) noexcept {
  if (sequence.size >= sequence.draws.size()) { sequence.overflow = true; return; }
  sequence.draws[sequence.size++] = draw;
}
bool vulkan_observed_missing_scene_matches(const VulkanObservedDrawSequence& sequence, unsigned available_mask) noexcept {
  if (available_mask > 3 || sequence.overflow || sequence.size > sequence.draws.size()) return false;
  VulkanObservedDrawSequence expected;
  append_vulkan_observed_draw(expected, {VulkanObservedDrawKind::indexed, 6, 1, 0, 0, 0});
  for (unsigned index = 0; index != 2; ++index)
    if (available_mask & (1U << index))
      append_vulkan_observed_draw(expected, {VulkanObservedDrawKind::direct, 6, 1, index * 6, 0, 0});
  append_vulkan_observed_draw(expected, {VulkanObservedDrawKind::indexed, 6, 1, 6, 0, 0});
  if (sequence.size != expected.size) return false;
  for (unsigned index = 0; index != sequence.size; ++index) {
    const auto& actual = sequence.draws[index];
    // The source index is firstVertex / 6, not an ordinal among available draws.
    if (actual.kind == VulkanObservedDrawKind::direct && actual.first % 6 != 0) return false;
    if (actual != expected.draws[index]) return false;
  }
  return true;
}
} // namespace cgpui_test
