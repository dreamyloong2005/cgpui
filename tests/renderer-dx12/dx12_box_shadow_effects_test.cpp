#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
Result<void> capture(RenderFrame& frame) {
  if (auto requested = frame.request_pixel_capture(); !requested) return requested;
  return frame.present();
}

bool logical_pixel(Renderer& renderer, float scale, float x, float y,
                   std::array<unsigned char, 4> expected) {
  return dx12_pixel(renderer.last_frame_pixels(), static_cast<unsigned>(x * scale),
                    static_cast<unsigned>(y * scale), expected);
}

int hard_effects(Renderer& renderer, float scale) {
  if (!renderer.resize({128, 128}, {scale})) return dx12_fail(10, "DPI resize failed");
  const PaintMetadata stack[] = {{.opacity = 0.8F},
      {.opacity = 0.5F, .transform = AffineTransform::translation(3, 5)}};
  const Rect clips[] = {{{0, 0}, {60, 64}}};
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(11, "hard effects begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{8, 8}, {16, 16}}, .radius = BorderRadii::all(4),
      .shadow = {.color = {1, 0, 0, 0.5F}, .offset = {32, 0}, .spread_radius = 4},
      .clip_rect = Rect{{0, 0}, {64, 64}}, .clip_stack = renderer_clip_stack_record(clips),
      .composition_stack = renderer_composition_stack_record(stack), .metadata = stack[1]});
  if (auto result = capture(**frame); !result) return dx12_fail(12, result.error().message.c_str());
  // Shape after spread+translation: [39,63)x[9,33), radius8. Alpha is .5*.5,
  // never multiplied by the earlier cumulative stack entry again.
  if (!logical_pixel(renderer, scale, 51, 21, {255, 191, 191, 255}) ||
      !logical_pixel(renderer, scale, 41, 21, {255, 191, 191, 255}) ||
      !logical_pixel(renderer, scale, 39, 9, {255, 255, 255, 255}) ||
      !logical_pixel(renderer, scale, 36, 21, {255, 255, 255, 255}) ||
      !logical_pixel(renderer, scale, 62, 21, {255, 255, 255, 255}) ||
      !logical_pixel(renderer, scale, 19, 21, {255, 255, 255, 255}))
    return dx12_fail(13, "radius/spread/opacity/clip/DPI differs");
  frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(14, "negative spread begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{8, 8}, {16, 16}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {32, 0}, .spread_radius = -4}});
  if (auto result = capture(**frame); !result) return dx12_fail(15, result.error().message.c_str());
  if (!logical_pixel(renderer, scale, 48, 16, {0, 0, 0, 255}) ||
      !logical_pixel(renderer, scale, 42, 16, {255, 255, 255, 255}) ||
      !logical_pixel(renderer, scale, 48, 22, {255, 255, 255, 255}))
    return dx12_fail(16, "negative spread differs");
  return 0;
}

// Independent analytic convolution of an axis-aligned rectangle by a normalized
// Gaussian. This is not the shader's erf approximation/four-tap integration.
double interval_mass(double point, double low, double high, double sigma) {
  const double inverse = 1.0 / (std::sqrt(2.0) * sigma);
  return 0.5 * (std::erf((high - point) * inverse) - std::erf((low - point) * inverse));
}

int blurred_falloff(Renderer& renderer, float scale) {
  if (!renderer.resize({128, 128}, {scale})) return dx12_fail(20, "blur resize failed");
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(21, "blur begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{24, 24}, {32, 32}},
      .shadow = {.color = {0, 0, 0, 1}, .blur_radius = 8}});
  if (auto result = capture(**frame); !result) return dx12_fail(22, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  const double low = 24 * scale, high = 56 * scale, sigma = 4 * scale;
  const auto middle = static_cast<unsigned>(40 * scale);
  int previous = 255;
  bool visible_falloff = false;
  for (unsigned x = 0; x < static_cast<unsigned>(low) - 1; ++x) {
    const double coverage = interval_mass(x + 0.5, low, high, sigma) *
                            interval_mass(middle + 0.5, low, high, sigma);
    const auto gray = static_cast<unsigned char>(std::lround(255 * (1 - coverage)));
    if (!dx12_pixel(pixels, x, middle, {gray, gray, gray, 255}, 3) ||
        !dx12_pixel(pixels, middle, x, {gray, gray, gray, 255}, 3))
      return dx12_fail(23, "blur falloff differs from independent Gaussian integral");
    const auto actual = pixels->pixel_rgba8(x, middle);
    if (!actual || (*actual)[0] > previous + 1) return dx12_fail(24, "blur falloff is not monotonic");
    previous = (*actual)[0];
    visible_falloff = visible_falloff || previous < 240;
  }
  return visible_falloff && dx12_pixel(pixels, middle, middle, {255, 255, 255, 255})
      ? 0 : dx12_fail(25, "blur invisible or original box was not clipped out");
}

int rounded_clipout(Renderer& renderer) {
  if (!renderer.resize({64, 64}, {1})) return dx12_fail(30, "rounded resize failed");
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(31, "rounded begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{16, 16}, {24, 24}}, .radius = BorderRadii::all(8),
      .shadow = {.color = {0, 0, 0, 1}, .spread_radius = 4}});
  if (auto result = capture(**frame); !result) return dx12_fail(32, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  return dx12_pixel(pixels, 16, 16, {0, 0, 0, 255}) &&
      dx12_pixel(pixels, 20, 20, {255, 255, 255, 255}) &&
      dx12_pixel(pixels, 14, 28, {0, 0, 0, 255}) &&
      dx12_pixel(pixels, 28, 28, {255, 255, 255, 255}) &&
      dx12_pixel(pixels, 12, 12, {255, 255, 255, 255})
      ? 0 : dx12_fail(33, "rounded original-box cutout differs");
}
}

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "effects window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  for (const float scale : {1.0F, 1.5F, 2.0F}) {
    if (const int result = hard_effects(**renderer, scale)) return result;
    if (const int result = blurred_falloff(**renderer, scale)) return result;
  }
  return rounded_clipout(**renderer);
}
