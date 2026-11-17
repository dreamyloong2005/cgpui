#include "vulkan_box_shadow_geometry_internal.hpp"
#include "vulkan_device_pixels_internal.hpp"
#include "vulkan_frame_draw_order_internal.hpp"
#include "vulkan_frame_pipeline_switch_internal.hpp"

#include <cmath>
#include <vector>

namespace {

bool close_to(float a, float b) { return std::abs(a - b) < 0.001F; }

cgpui::BoxShadowDraw sample_shadow() {
  return cgpui::BoxShadowDraw{
      .rect = {.origin = {10, 20}, .size = {30, 40}},
      .radius = cgpui::BorderRadii::all(6.0F),
      .shadow = {.color = {.a = 0.8F}, .offset = {2, 3}, .blur_radius = 8,
                 .spread_radius = 4},
      .metadata = {.opacity = 0.5F},
  };
}

// CSS conventions: sigma = blur / 2; spread grows radii (with the
// css-backgrounds-3 cubic adjustment below the spread) and keeps 0 sharp.
int test_conventions() {
  if (!close_to(cgpui::vulkan_box_shadow_sigma(8.0F), 4.0F) ||
      !close_to(cgpui::vulkan_box_shadow_sigma(-2.0F), 0.0F) ||
      !close_to(cgpui::vulkan_box_shadow_spread_radius(0.0F, 10.0F), 0.0F) ||
      !close_to(cgpui::vulkan_box_shadow_spread_radius(12.0F, 10.0F), 22.0F) ||
      !close_to(cgpui::vulkan_box_shadow_spread_radius(5.0F, 10.0F), 13.75F) ||
      !close_to(cgpui::vulkan_box_shadow_spread_radius(5.0F, -8.0F), 0.0F)) {
    return 1;
  }
  return 0;
}

// One quad per shadow covering shape + 3 sigma + 1, opacity applied.
int test_geometry() {
  const std::vector shadows{sample_shadow()};
  const auto geometry = cgpui::vulkan_build_box_shadow_geometry(shadows);
  if (geometry.vertices.size() != 4 || geometry.indices.size() != 6 ||
      geometry.draws.size() != 1 || geometry.draws.front().source_index != 0) {
    return 10;
  }
  const auto& vertex = geometry.vertices.front();
  // Shape: (10 + 2 - 4, 20 + 3 - 4) size 38 x 48; margin 3 * 4 + 1.
  if (!close_to(vertex.shadow_rect[0], 8.0F) || !close_to(vertex.shadow_rect[3], 67.0F) ||
      !close_to(vertex.local[0], -5.0F) || !close_to(vertex.local[1], 6.0F) ||
      !close_to(vertex.shadow_radii[0], 10.0F) || !close_to(vertex.box_radii[2], 6.0F) ||
      !close_to(vertex.box_rect[2], 40.0F) || !close_to(vertex.sigma, 4.0F) ||
      !close_to(vertex.color[3], 0.4F)) {
    return 11;
  }
  cgpui::BoxShadowDraw empty = sample_shadow();
  empty.shadow.spread_radius = -20.0F;
  const std::vector skipped{empty};
  return cgpui::vulkan_build_box_shadow_geometry(skipped).draws.empty() ? 0
                                                                        : 12;
}

// Logical lengths become device pixels at the surface scale.
int test_device_pixels() {
  const auto device = cgpui::vulkan_to_device_pixels(
      sample_shadow(), cgpui::DpiScale{1.5F});
  return close_to(device.rect.origin.x, 15.0F) &&
                 close_to(device.radius.top_left, 9.0F) &&
                 close_to(device.shadow.offset.y, 4.5F) &&
                 close_to(device.shadow.blur_radius, 12.0F) &&
                 close_to(device.shadow.spread_radius, 6.0F)
             ? 0
             : 20;
}

// The draw order cursor resolves shadows between other draws, and the
// shadow pipeline is its own switch target.
int test_draw_order() {
  const std::vector<cgpui::VulkanFrameDrawOrderEntry> order{
      {.primitive_kind = cgpui::RendererPrimitiveKind::box_shadow},
      {.primitive_kind = cgpui::RendererPrimitiveKind::rounded_rect},
  };
  const std::vector<cgpui::VulkanRoundedRectDrawRange> rounded{{}};
  const std::vector<cgpui::VulkanRoundedRectDrawRange> shadows{{}};
  cgpui::VulkanFrameDrawOrderCursor cursor(order, {}, rounded, {}, {}, shadows);
  const auto first = cursor.next();
  const auto second = cursor.next();
  if (!first || !second || cursor.next() ||
      first->resource_kind != cgpui::VulkanFrameDrawResourceKind::box_shadow ||
      second->resource_kind != cgpui::VulkanFrameDrawResourceKind::rounded_rect) {
    return 30;
  }
  cgpui::VulkanFramePipelineSwitchState state;
  (void)cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::box_shadow);
  const auto plan = cgpui::vulkan_plan_frame_pipeline_switch(
      state, cgpui::VulkanFrameDrawResourceKind::rounded_rect);
  return plan.bind_pipeline ? 0 : 31;
}

} // namespace

int main() {
  for (const int result : {test_conventions(), test_geometry(),
                           test_device_pixels(), test_draw_order()}) {
    if (result != 0) return result;
  }
  return 0;
}
