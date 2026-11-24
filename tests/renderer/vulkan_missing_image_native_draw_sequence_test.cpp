#include "vulkan_missing_image_native_draw_sequence_test_support.hpp"
#include <utility>

using namespace cgpui_test;
namespace {
VulkanObservedDrawSequence scene(unsigned mask) {
  VulkanObservedDrawSequence sequence;
  append_vulkan_observed_draw(sequence, {VulkanObservedDrawKind::indexed, 6, 1, 0, 0, 0});
  if (mask & 1) append_vulkan_observed_draw(sequence, {VulkanObservedDrawKind::direct, 6, 1, 0, 0, 0});
  if (mask & 2) append_vulkan_observed_draw(sequence, {VulkanObservedDrawKind::direct, 6, 1, 6, 0, 0});
  append_vulkan_observed_draw(sequence, {VulkanObservedDrawKind::indexed, 6, 1, 6, 0, 0});
  return sequence;
}
}
int main() {
  for (unsigned mask = 0; mask != 4; ++mask) {
    const auto original = scene(mask);
    if (!vulkan_observed_missing_scene_matches(original, mask)) return 1;
    for (unsigned other = 0; other != 4; ++other)
      if (other != mask && vulkan_observed_missing_scene_matches(original, other)) return 2;
    for (unsigned index = 0; index != original.size; ++index) {
      for (unsigned field = 0; field != 6; ++field) {
        auto changed = original;
        auto& draw = changed.draws[index];
        if (field == 0) draw.kind = draw.kind == VulkanObservedDrawKind::indexed
            ? VulkanObservedDrawKind::direct : VulkanObservedDrawKind::indexed;
        if (field == 1) ++draw.count;
        if (field == 2) ++draw.instances;
        if (field == 3) ++draw.first;
        if (field == 4) ++draw.vertex_offset;
        if (field == 5) ++draw.first_instance;
        if (vulkan_observed_missing_scene_matches(changed, mask)) return 3;
      }
    }
    auto changed = original;
    append_vulkan_observed_draw(changed, original.draws[0]);
    if (vulkan_observed_missing_scene_matches(changed, mask)) return 4;
    changed = original; --changed.size;
    if (vulkan_observed_missing_scene_matches(changed, mask)) return 5;
    changed = original; changed.overflow = true;
    if (vulkan_observed_missing_scene_matches(changed, mask)) return 6;
  }
  auto first_only = scene(1); first_only.draws[1].first = 6; // Original P2 witness.
  if (vulkan_observed_missing_scene_matches(first_only, 1)) return 7;
  auto both = scene(3); std::swap(both.draws[1], both.draws[2]);
  if (vulkan_observed_missing_scene_matches(both, 3)) return 8;
  auto reordered = scene(1); std::swap(reordered.draws[1], reordered.draws[2]);
  if (vulkan_observed_missing_scene_matches(reordered, 1)) return 9;
  auto duplicated = scene(3); duplicated.draws[2] = duplicated.draws[1];
  if (vulkan_observed_missing_scene_matches(duplicated, 3)) return 10;
  return vulkan_observed_missing_scene_matches(scene(0), 4) ? 11 : 0;
}
