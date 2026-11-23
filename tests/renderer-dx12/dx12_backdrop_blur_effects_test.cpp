#include "dx12_test_support.hpp"

#include <numbers>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int gaussian_edges(Renderer& renderer) {
  if (!renderer.resize({1024, 128}, {1})) return dx12_fail(10, "edge resize failed");
  // These logical sigmas cover the four quality/downsample ranges. The oracle
  // is the continuous Gaussian CDF of a step, independent of tap construction,
  // texture layout, resampling implementation and the neutral planner.
  for (const float sigma : {4.0F, 12.0F, 32.0F, 64.0F}) {
    auto frame = renderer.begin_frame();
    if (!frame) return dx12_fail(11, "edge begin failed");
    (*frame)->clear({1, 1, 1, 1});
    (*frame)->draw_rect({.rect = {{0, 0}, {512, 128}}, .color = {0, 0, 0, 1}});
    (*frame)->draw_backdrop_blur({.rect = {{16, 16}, {992, 96}}, .blur_radius = sigma});
    if (!(*frame)->request_pixel_capture()) return dx12_fail(12, "edge capture refused");
    if (auto result = (*frame)->present(); !result)
      return dx12_fail(13, result.error().message.c_str());
    int previous = -1;
    for (const double distance : {-2.0, -1.0, -0.25, 0.25, 1.0, 2.0}) {
      const auto x = static_cast<unsigned>(512 + distance * sigma);
      const double z = (x + 0.5 - 512) / (sigma * std::numbers::sqrt2);
      const auto value = static_cast<unsigned char>(std::lround(127.5 * (1 + std::erf(z))));
      // Five bytes cover finite-kernel truncation, resampling and RGBA8
      // intermediate quantization; this budget is not fitted to GPU results.
      if (!dx12_pixel(renderer.last_frame_pixels(), x, 64, {value, value, value, 255}, 5))
        return dx12_fail(14, "blur sigma differs from independent Gaussian step integral");
      const auto pixel = renderer.last_frame_pixels()->pixel_rgba8(x, 64);
      if (!pixel || (*pixel)[0] <= previous || (*pixel)[3] != 255)
        return dx12_fail(15, "blur edge must be monotone and opaque");
      previous = (*pixel)[0];
    }
    if (!dx12_pixel(renderer.last_frame_pixels(), 511, 4, {0, 0, 0, 255}) ||
        !dx12_pixel(renderer.last_frame_pixels(), 512, 4, {255, 255, 255, 255}))
      return dx12_fail(16, "blur modified pixels outside its panel");
  }
  return 0;
}

int rounded_tint_clip(Renderer& renderer, float scale) {
  if (!renderer.resize({128, 128}, {scale})) return dx12_fail(20, "mask DPI resize failed");
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(21, "mask begin failed");
  (*frame)->clear({0, 0, 1, 1});
  BackdropBlurDraw blur{.rect = {{8, 8}, {24, 24}}, .radius = BorderRadii::all(8),
      .blur_radius = 4, .tint = Color{1, 0, 0, 0.5F},
      .clip_rect = Rect{{0, 0}, {36, 60}}, .metadata = {.opacity = 0.2F}};
  blur.clip_stack.clips = {Rect{{0, 0}, {32, 60}}};
  blur.composition_stack = {
      .entries = {PaintMetadata{.opacity = 0.8F}, PaintMetadata{.opacity = 0.5F}},
      .full_depth = 2,
      .current_metadata = {.opacity = 0.5F, .transform = {.translate_x = 8, .translate_y = 4}}};
  (*frame)->draw_backdrop_blur(blur);
  if (!(*frame)->request_pixel_capture()) return dx12_fail(22, "mask capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(23, result.error().message.c_str());
  const auto sample = [&](unsigned x, unsigned y, std::array<unsigned char, 4> expected) {
    return dx12_pixel(renderer.last_frame_pixels(), unsigned(x * scale), unsigned(y * scale), expected);
  };
  // Uniform blue is invariant under any normalized blur. Half-red tint gives
  // purple; compositing the effect at cumulative opacity .5 gives 1/4 red,
  // 3/4 blue. The translated rounded panel is [16,40)x[12,36), clipped at x32.
  if (!sample(20, 24, {64, 0, 191, 255}) ||
      !sample(16, 12, {0, 0, 255, 255}) ||
      !sample(12, 24, {0, 0, 255, 255}) ||
      !sample(34, 24, {0, 0, 255, 255}) ||
      !sample(38, 24, {0, 0, 255, 255}) ||
      !sample(20, 40, {0, 0, 255, 255}))
    return dx12_fail(24, "rounded tint, cumulative opacity, translation, clip or DPI differs");
  return 0;
}
} // namespace

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "effects window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  if (!(*renderer)->backdrop_blur_support().supported)
    return dx12_fail(3, "DX12 must advertise its native backdrop support");
  if (const int result = gaussian_edges(**renderer)) return result;
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    if (const int result = rounded_tint_clip(**renderer, scale)) return result;
  }
  return 0;
}
