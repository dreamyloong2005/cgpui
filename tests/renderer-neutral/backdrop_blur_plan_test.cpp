#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"
#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {
bool near(float a, float b) { return std::abs(a - b) < 0.00001F; }
int fail(int code, const char* message) {
  std::cerr << "backdrop planning: " << message << '\n';
  return code;
}
}

int main() {
  using namespace cgpui;
  static_assert(std::is_standard_layout_v<BackdropBlurVertex>);
  static_assert(std::is_same_v<decltype(BackdropBlurPass::region), PixelScissor>);
  static_assert(std::is_same_v<decltype(BackdropBlurPass::texels), PixelExtent>);
  BackdropBlurDraw backdrop{
      .rect = {{20, 30}, {100, 50}}, .radius = BorderRadii::all(8),
      .blur_radius = 10, .tint = Color{1, 0, 0, 0.25F},
      .clip_rect = Rect{{15, 25}, {30, 40}}, .metadata = {.opacity = 0.25F},
  };
  backdrop.clip_stack.clips = {Rect{{20, 0}, {20, 50}}};
  backdrop.composition_stack = {
      .entries = {PaintMetadata{.opacity = 0.5F}}, .full_depth = 1,
      .current_metadata = {.opacity = 0.5F,
          .transform = AffineTransform::translation(5, -10)},
  };
  BackdropBlurDraw offscreen = backdrop;
  offscreen.rect.origin = {1000, 1000};
  BackdropBlurDraw empty = backdrop;
  empty.rect.size.width = 0;
  const auto plan = plan_backdrop_blur(std::array{offscreen, backdrop, empty}, {200, 100});
  if (plan.passes.size() != 1 || plan.vertices.size() != 6 ||
      kBackdropBlurVerticesPerPass != 6 || kBackdropBlurMaxRadius != 48) {
    return fail(1, "visible backdrop must produce one six-vertex pass");
  }
  const auto& pass = plan.passes[0];
  // Transformed AA quad is (24,19)-(126,71); 30px blur margin is frame-clamped.
  if (pass.source_index != 1 || pass.first_vertex != 0 ||
      pass.region.x != 0 || pass.region.y != 0 || pass.region.width != 156 ||
      pass.region.height != 100 || pass.factor != 2 || pass.texels.width != 78 ||
      pass.texels.height != 50 || !near(pass.sigma, 4.99374609F) || pass.radius != 15) {
    return fail(2, "region/downsample/kernel differs from the worked example");
  }
  if (!pass.clip_rect || pass.clip_rect->origin.x != 20 || pass.clip_rect->origin.y != 25 ||
      pass.clip_rect->size.width != 20 || pass.clip_rect->size.height != 25) {
    return fail(3, "clip stack intersection differs");
  }
  const auto& vertex = plan.vertices[0];
  if (vertex.position != std::array<float, 2>{24, 19} ||
      vertex.local != std::array<float, 2>{19, 29} ||
      vertex.rect != std::array<float, 4>{20, 30, 120, 80} ||
      vertex.radii != std::array<float, 4>{8, 8, 8, 8} ||
      vertex.tint != std::array<float, 4>{1, 0, 0, 0.25F} ||
      vertex.region != std::array<float, 4>{0, 0, 2, 0.5F} ||
      plan.vertices[2].position != std::array<float, 2>{126, 71} ||
      plan.vertices[4].position != std::array<float, 2>{24, 71}) {
    return fail(4, "composite local/device geometry or color metadata differs");
  }
  backdrop.blur_radius = std::numeric_limits<float>::quiet_NaN();
  backdrop.tint.reset();
  const auto unblurred = plan_backdrop_blur(std::array{backdrop}, {200, 100});
  if (unblurred.passes.size() != 1 || unblurred.passes[0].factor != 1 ||
      unblurred.passes[0].sigma != 0 || unblurred.passes[0].radius != 0 ||
      unblurred.vertices[0].tint[3] != 0) {
    return fail(5, "nonfinite blur or absent tint fallback differs");
  }
  if (backdrop_blur_downsample_factor(4) != 1 ||
      backdrop_blur_downsample_factor(4.001F) != 2 ||
      backdrop_blur_downsample_factor(16) != 2 ||
      backdrop_blur_downsample_factor(16.001F) != 4 ||
      backdrop_blur_downsample_factor(48) != 4 ||
      backdrop_blur_downsample_factor(48.001F) != 8 ||
      !plan_backdrop_blur({}, {200, 100}).passes.empty() ||
      !plan_backdrop_blur(std::array{backdrop}, {0, 100}).passes.empty()) {
    return fail(6, "downsample thresholds or empty-frame planning differs");
  }
  return 0;
}
