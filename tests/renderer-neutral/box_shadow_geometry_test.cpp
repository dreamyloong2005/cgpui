#include "cgpui/renderer/planning/box_shadow_geometry.hpp"
#include "cgpui/renderer/planning/box_shadow_parameters.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {
bool near(float a, float b) { return std::abs(a - b) < 0.0001F; }
int fail(int code, const char* message) {
  std::cerr << "shadow planning: " << message << '\n';
  return code;
}
}

int main() {
  using namespace cgpui;
  static_assert(std::is_standard_layout_v<BoxShadowVertex>);
  static_assert(std::is_trivially_copyable_v<BoxShadowVertex>);
  static_assert(sizeof(BoxShadowVertex) == 25 * sizeof(float));
  const float nan = std::numeric_limits<float>::quiet_NaN();
  if (!near(box_shadow_sigma(8), 4) || box_shadow_sigma(-2) != 0 ||
      box_shadow_sigma(nan) != 0 || box_shadow_spread_radius(0, 10) != 0 ||
      !near(box_shadow_spread_radius(5, 10), 13.75F) ||
      !near(box_shadow_spread_radius(12, 10), 22) ||
      box_shadow_spread_radius(5, -8) != 0) return fail(1, "CSS blur/spread differs");
  BoxShadowDraw shadow{
      .rect = {{10, 20}, {30, 40}}, .radius = BorderRadii::all(6),
      .shadow = {.color = {0.2F, 0.4F, 0.6F, 0.8F}, .offset = {2, 3},
                 .blur_radius = 8, .spread_radius = 4},
      .clip_rect = Rect{{0, 10}, {30, 50}}, .metadata = {.opacity = 0.5F},
  };
  shadow.clip_stack.clips = {Rect{{5, 0}, {20, 40}}};
  shadow.composition_stack = {
      .entries = {PaintMetadata{.opacity = 0.5F}, PaintMetadata{.opacity = 0.25F}},
      .full_depth = 2,
      .current_metadata = {.opacity = 0.25F,
          .transform = {.scale_x = 2, .scale_y = 3, .translate_x = 5, .translate_y = 7}},
  };
  BoxShadowDraw collapsed = shadow;
  collapsed.shadow.spread_radius = -20;
  BoxShadowDraw invisible = shadow;
  invisible.shadow.color.a = 0;
  const auto geometry = build_box_shadow_geometry(std::array{collapsed, shadow, invisible});
  if (geometry.vertices.size() != 4 || geometry.indices !=
          std::vector<std::uint32_t>{0, 1, 2, 2, 3, 0} || geometry.draws.size() != 1) {
    return fail(2, "quad expansion or degenerate filtering differs");
  }
  const auto& vertex = geometry.vertices[0];
  if (vertex.local != std::array<float, 2>{-5, 6} ||
      vertex.position != std::array<float, 2>{-5, 25} ||
      geometry.vertices[2].local != std::array<float, 2>{59, 80} ||
      geometry.vertices[2].position != std::array<float, 2>{123, 247} ||
      vertex.shadow_rect != std::array<float, 4>{8, 19, 46, 67} ||
      vertex.box_rect != std::array<float, 4>{10, 20, 40, 60} ||
      vertex.shadow_radii != std::array<float, 4>{10, 10, 10, 10} ||
      vertex.box_radii != std::array<float, 4>{6, 6, 6, 6} ||
      !near(vertex.sigma, 4) || !near(vertex.color[3], 0.2F)) {
    return fail(3, "shadow shape/local transform/opacity differs");
  }
  const auto& draw = geometry.draws[0];
  if (draw.source_index != 1 || draw.first_vertex != 0 || draw.vertex_count != 4 ||
      draw.first_index != 0 || draw.index_count != 6 || !draw.clip_rect ||
      draw.clip_rect->origin.x != 5 || draw.clip_rect->origin.y != 10 ||
      draw.clip_rect->size.width != 20 || draw.clip_rect->size.height != 30) {
    return fail(4, "source mapping or clip intersection differs");
  }
  shadow.shadow.blur_radius = nan;
  shadow.shadow.spread_radius = nan;
  shadow.radius = BorderRadii::all(100);
  const auto sanitized = build_box_shadow_geometry(std::array{shadow});
  if (sanitized.vertices.size() != 4 || sanitized.vertices[0].sigma != 0 ||
      sanitized.vertices[0].shadow_rect != std::array<float, 4>{12, 23, 42, 63} ||
      !near(sanitized.vertices[0].box_radii[0], 15) ||
      !near(sanitized.vertices[0].box_radii[1], 15) ||
      !near(sanitized.vertices[0].box_radii[2], 15) ||
      !near(sanitized.vertices[0].box_radii[3], 15) ||
      !build_box_shadow_geometry({}).vertices.empty()) {
    return fail(5, "invalid blur/spread or radius normalization differs");
  }
  return 0;
}
