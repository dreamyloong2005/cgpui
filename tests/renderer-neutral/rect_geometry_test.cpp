#include "cgpui/renderer/planning/rect_geometry.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include "cgpui/renderer/planning/composition.hpp"

#include <array>
#include <cmath>
#include <limits>

namespace {
bool near(float a, float b) { return std::abs(a - b) < 0.0001F; }
}

int main() {
  using namespace cgpui;
  SolidRect solid{.rect = {{2, 3}, {10, 8}}, .color = {1, 0.5F, 0.25F, 0.8F}};
  solid.metadata.opacity = 0.5F;
  solid.metadata.transform = {.scale_x = 2, .scale_y = 3, .translate_x = 5, .translate_y = 7};
  solid.clip_rect = Rect{{1.25F, 2.5F}, {8, 10}};
  const auto geometry = build_solid_rect_geometry(std::array{solid, SolidRect{}});
  if (geometry.vertices.size() != 4 || geometry.indices.size() != 6 || geometry.draws.size() != 1) return 1;
  if (geometry.draws[0].source_index != 0 || geometry.indices != std::vector<std::uint32_t>{0, 1, 2, 0, 2, 3}) return 2;
  if (!near(geometry.vertices[0].position[0], 9) || !near(geometry.vertices[0].position[1], 16) ||
      !near(geometry.vertices[2].position[0], 29) || !near(geometry.vertices[2].position[1], 40) ||
      !near(geometry.vertices[0].color[3], 0.4F)) return 3;
  RendererClipStackRecord clips;
  clips.clips.push_back(Rect{{4, 0}, {20, 6}});
  const auto scissor = resolve_clip_stack_scissor({100, 50}, solid.clip_rect, clips);
  if (!scissor.visible || !scissor.clipped || scissor.scissor.x != 4 || scissor.scissor.y != 2 ||
      scissor.scissor.width != 6 || scissor.scissor.height != 4) return 4;
  if (resolve_clip_stack_scissor({0, 50}, std::nullopt, {}).visible ||
      resolve_clip_stack_scissor({100, 50}, Rect{{200, 0}, {4, 4}}, {}).visible) return 5;
  RoundedRectDraw rounded{.rect = {{0, 0}, {20, 10}}, .color = {1, 0, 0, 1},
                          .radius = BorderRadii::all(2)};
  const auto fill = build_rounded_rect_geometry(std::array{rounded}, 1);
  if (fill.vertices.size() != 17 || fill.indices.size() != 72 || fill.draws.size() != 1) return 6;
  if (!near(fill.vertices[0].position[0], 10) || !near(fill.vertices[0].position[1], 5) ||
      fill.vertices[1].coverage != 1 || fill.vertices.back().coverage != 0) return 7;
  rounded.fill_enabled = false;
  rounded.border_color = Color{0, 1, 0, 0.5F};
  rounded.border_width = 1;
  const auto stroke = build_rounded_rect_geometry(std::array{rounded}, 1);
  if (stroke.vertices.size() != 24 || stroke.indices.size() != 96 || stroke.draws.size() != 1) return 8;
  for (const auto index : stroke.indices) if (index >= stroke.vertices.size()) return 9;
  PaintMetadata invalid;
  invalid.opacity = std::numeric_limits<float>::quiet_NaN();
  invalid.transform.translate_x = std::numeric_limits<float>::infinity();
  if (resolve_composed_opacity(invalid, {}) != 1 ||
      apply_composed_transform({3, 4}, invalid, {}).x != 3) return 10;
  return 0;
}
