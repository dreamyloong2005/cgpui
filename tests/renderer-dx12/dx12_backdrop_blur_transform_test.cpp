#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

#include <numbers>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct AffineCase {
  AffineTransform transform;
  Point outside_black, outside_white;
};

int transformed_edge(Renderer& renderer, const AffineCase& test, float scale) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(10, "affine backdrop begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_rect({.rect = {{0, 0}, {64, 64}}, .color = {0, 0, 0, 1}});
  (*frame)->draw_backdrop_blur({.rect = {{8, 8}, {32, 24}}, .blur_radius = 4,
      .metadata = {.transform = test.transform}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "affine capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());
  // The mask transforms, but the backdrop is the already-painted SCREEN scene.
  // Both points are inside every hand-computed transformed panel below. Their
  // Gaussian values depend on screen x, not the inverse-transformed local x.
  for (const unsigned logical_x : {63U, 67U}) {
    const unsigned x = unsigned(logical_x * scale), y = unsigned(24 * scale);
    const double screen_x = (x + 0.5) / scale;
    const double z = (screen_x - 64) / (4 * std::numbers::sqrt2);
    const auto value = static_cast<unsigned char>(std::lround(127.5 * (1 + std::erf(z))));
    // Same fixed approximation budget as the independent non-affine edge test.
    if (!dx12_pixel(renderer.last_frame_pixels(), x, y, {value, value, value, 255}, 5))
      return dx12_fail(13, "affine backdrop sampled local coordinates or lost DPI sigma scaling");
  }
  if (!dx12_pixel(renderer.last_frame_pixels(), unsigned(test.outside_black.x * scale),
                  unsigned(test.outside_black.y * scale), {0, 0, 0, 255}) ||
      !dx12_pixel(renderer.last_frame_pixels(), unsigned(test.outside_white.x * scale),
                  unsigned(test.outside_white.y * scale), {255, 255, 255, 255}))
    return dx12_fail(14, "affine backdrop mask changed pixels outside transformed panel");
  return 0;
}
} // namespace

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "affine backdrop window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const AffineCase cases[] = {
      // Rotation: [8,40)x[8,32) becomes [56,80)x[8,40).
      {{.scale_x = 0, .skew_y = 1, .skew_x = -1, .scale_y = 0, .translate_x = 88},
       {63, 44}, {67, 44}},
      // Nonuniform scale: [44,92)x[14,32). The scene's blur stays sigma 4.
      {{.scale_x = 1.5F, .scale_y = 0.75F, .translate_x = 32, .translate_y = 8},
       {63, 38}, {67, 38}},
      // Shear: at y24 the panel spans x[60,92); at y38 x[67,99).
      // Both outside probes lie inside its AABB, but outside its actual shape.
      {{.skew_x = 0.5F, .translate_x = 44, .translate_y = 8},
       {60, 36}, {65, 38}},
  };
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    if (!(*renderer)->resize({128 * scale, 64 * scale}, {scale}))
      return dx12_fail(3, "affine backdrop DPI resize failed");
    for (const auto& test : cases) {
      if (const int result = transformed_edge(**renderer, test, scale)) return result;
    }
  }
  return 0;
}
