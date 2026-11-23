#include "dx12_test_support.hpp"

#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
BackdropBlurDraw tinted_panel(float sigma) {
  return {.rect = {{8, 8}, {48, 48}}, .blur_radius = sigma,
          .tint = Color{1, 0, 0, 0.5F}};
}

int check_zero_sigma(Renderer& renderer, float sigma) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, "invalid backdrop poisoned the next frame");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_rect({.rect = {{0, 0}, {32, 64}}, .color = {0, 0, 0, 1}});
  (*frame)->draw_backdrop_blur(tinted_panel(sigma));
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "normalized capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  // Zero sigma preserves the hard edge. Half-red tint mixes with black/white
  // independently on either side; no planner or GPU kernel defines this oracle.
  return dx12_pixel(pixels, 31, 32, {128, 0, 0, 255}) &&
      dx12_pixel(pixels, 32, 32, {255, 128, 128, 255}) &&
      dx12_pixel(pixels, 31, 4, {0, 0, 0, 255}) &&
      dx12_pixel(pixels, 32, 4, {255, 255, 255, 255})
      ? 0 : dx12_fail(13, "normalized sigma blurred the edge or lost tint/panel bounds");
}

int check_extreme_sigma(Renderer& renderer) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(20, "extreme sigma begin failed");
  (*frame)->clear({0.25F, 0.5F, 0.75F, 1});
  (*frame)->draw_backdrop_blur({.rect = {{8, 8}, {48, 48}},
      .blur_radius = (std::numeric_limits<float>::max)()});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(21, "extreme capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(22, result.error().message.c_str());
  // Every normalized blur preserves a uniform field, even at the maximum
  // finite public sigma. This does not prescribe a capped kernel's shape.
  for (const auto point : {Point{16, 16}, Point{32, 32}, Point{48, 48}, Point{4, 4}}) {
    if (!dx12_pixel(renderer.last_frame_pixels(), unsigned(point.x), unsigned(point.y),
                    {64, 128, 191, 255}))
      return dx12_fail(23, "maximum finite sigma changed a uniform color");
  }
  return 0;
}
} // namespace

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "validation backdrop window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float infinity = std::numeric_limits<float>::infinity();
  // The shared effect policy normalizes nonfinite/negative sigma to zero;
  // unlike raw bounds/tint/clip data, these values must not reject the frame.
  for (const float sigma : {nan, infinity, -infinity, -4.0F, 0.0F}) {
    if (const int result = check_zero_sigma(**renderer, sigma)) return result;
  }
  for (unsigned invalid_case = 0; invalid_case < 10; ++invalid_case) {
    auto invalid = tinted_panel(4);
    switch (invalid_case) {
      case 0: invalid.rect.origin.x = nan; break;
      case 1: invalid.rect.origin.y = -infinity; break;
      case 2: invalid.rect.size.width = infinity; break;
      case 3: invalid.rect.size.height = nan; break;
      case 4: invalid.tint->r = nan; break;
      case 5: invalid.tint->a = infinity; break;
      case 6: invalid.clip_rect = Rect{{nan, 0}, {64, 64}}; break;
      case 7: invalid.clip_rect = Rect{{0, 0}, {64, infinity}}; break;
      case 8: invalid.clip_stack.current_clip_rect = Rect{{0, -infinity}, {64, 64}}; break;
      case 9: invalid.clip_stack.clips = {Rect{{0, 0}, {nan, 64}}}; break;
    }
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(3, "invalid backdrop begin failed");
    (*frame)->draw_backdrop_blur(invalid);
    const auto result = (*frame)->present();
    if (result || result.error().code != ErrorCode::invalid_argument)
      return dx12_fail(4, "nonfinite backdrop raw input accepted or wrong error");
    if (const int recovered = check_zero_sigma(**renderer, 0)) return recovered;
  }
  return check_extreme_sigma(**renderer);
}
