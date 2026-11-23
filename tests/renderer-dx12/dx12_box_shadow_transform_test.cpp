#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

#include <algorithm>
#include <numbers>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int affine_cases(Renderer& renderer, float scale) {
  struct Case {
    AffineTransform transform;
    Point black, white;
  };
  const Case cases[] = {
      // A 90-degree rotation maps shadow [24,36)x[4,16) to [40,52)x[24,36).
      {{.scale_x = 0, .skew_y = 1, .skew_x = -1, .scale_y = 0, .translate_x = 56},
       {46, 30}, {46, 10}},
      {{.scale_x = 1.5F, .scale_y = 0.5F, .translate_x = 4, .translate_y = 8},
       {49, 13}, {49, 19}},
      {{.skew_x = 0.5F, .translate_x = 2, .translate_y = 3},
       {35, 12}, {24, 12}},
  };
  for (const auto& test : cases) {
    auto frame = cgpui_test::wait_for_test_frame(renderer);
    if (!frame) return dx12_fail(10, "affine shadow begin failed");
    (*frame)->clear({1, 1, 1, 1});
    (*frame)->draw_box_shadow({.rect = {{4, 4}, {12, 12}},
        .shadow = {.color = {0, 0, 0, 1}, .offset = {20, 0}},
        .metadata = {.transform = test.transform}});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "affine capture refused");
    if (auto result = (*frame)->present(); !result) return dx12_fail(12, result.error().message.c_str());
    const auto* pixels = renderer.last_frame_pixels();
    if (!dx12_pixel(pixels, unsigned(test.black.x * scale), unsigned(test.black.y * scale), {0, 0, 0, 255}) ||
        !dx12_pixel(pixels, unsigned(test.white.x * scale), unsigned(test.white.y * scale), {255, 255, 255, 255}))
      return dx12_fail(13, "rotation/nonuniform-scale/shear differs");
  }
  return 0;
}

// Independent 2D midpoint integration over the actual rounded shape. It does
// not use shader distance/erf approximations, four-tap quadrature or CPU geometry.
double rounded_gaussian(double px, double py, double step) {
  constexpr double sigma = 4, radius = 8;
  double sum = 0;
  for (double y = 24 + step / 2; y < 56; y += step) {
    for (double x = 24 + step / 2; x < 56; x += step) {
      const double cx = std::clamp(x, 32.0, 48.0), cy = std::clamp(y, 32.0, 48.0);
      if ((x - cx) * (x - cx) + (y - cy) * (y - cy) > radius * radius) continue;
      const double dx = x - px, dy = y - py;
      sum += std::exp(-(dx * dx + dy * dy) / (2 * sigma * sigma));
    }
  }
  return sum * step * step / (2 * std::numbers::pi * sigma * sigma);
}

int rounded_soft(Renderer& renderer, float scale) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(20, "rounded soft begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{24, 24}, {32, 32}}, .radius = BorderRadii::all(8),
      .shadow = {.color = {0, 0, 0, 1}, .blur_radius = 8}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(21, "rounded soft capture refused");
  if (auto result = (*frame)->present(); !result) return dx12_fail(22, result.error().message.c_str());
  for (const Point point : {Point{20, 20}, {22, 22}, {24, 24}, {26, 24}, {24, 26}}) {
    const unsigned x = unsigned(point.x * scale), y = unsigned(point.y * scale);
    const double px = (x + 0.5) / scale, py = (y + 0.5) / scale;
    const double coarse = rounded_gaussian(px, py, 0.125);
    const double coverage = rounded_gaussian(px, py, 0.0625);
    if (std::abs(coarse - coverage) * 255 > 0.25)
      return dx12_fail(24, "independent rounded integral has not converged to sub-byte precision");
    const auto value = static_cast<unsigned char>(std::lround(255 * (1 - coverage)));
    // Three bytes allows the existing analytic shader's documented approximation,
    // not numerical-reference noise (separately constrained above).
    if (!dx12_pixel(renderer.last_frame_pixels(), x, y, {value, value, value, 255}, 3))
      return dx12_fail(23, "rounded blur differs from independent Gaussian area integral");
  }
  return 0;
}
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "transform window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    if (!(*renderer)->resize({128, 128}, {scale})) return dx12_fail(3, "transform DPI resize failed");
    if (const int result = affine_cases(**renderer, scale)) return result;
    if (const int result = rounded_soft(**renderer, scale)) return result;
  }
  return 0;
}
