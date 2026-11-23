#include "dx12_popup_backdrop_test_support.hpp"

#include <numbers>

using namespace cgpui;
using namespace cgpui_test;

namespace {
int check_step(Renderer& renderer, float sigma, float opacity) {
  std::cout << "popup alpha step: sigma " << sigma << ", opacity " << opacity << '\n';
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 0});
  (*frame)->draw_rect({.rect = {{512, 0}, {512, 128}}, .color = {1, 0, 0, 1}});
  (*frame)->draw_backdrop_blur({.rect = {{16, 16}, {992, 96}}, .blur_radius = sigma,
      .metadata = {.opacity = opacity}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "alpha step capture refused");
  if (const auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || pixels->width != 1024 || pixels->height != 128)
    return dx12_fail(13, "alpha step framebuffer capture unavailable");

  int previous = -1;
  for (const double distance : {-2.0, -1.0, -0.25, 0.25, 1.0, 2.0}) {
    const auto x = static_cast<unsigned>(512 + distance * sigma);
    // Independent continuous Gaussian integral of an opaque-red/transparent
    // step. The scene and filtered result are premultiplied, so R equals A.
    // Partial effect opacity interpolates ORIGINAL scene with filtered scene;
    // source-over would incorrectly keep the right side's alpha at one.
    const double z = (x + 0.5 - 512) / (sigma * std::numbers::sqrt2);
    const double filtered = 0.5 * (1 + std::erf(z));
    const double original = x >= 512 ? 1 : 0;
    const auto expected = static_cast<unsigned char>(std::lround(
        255 * (original * (1 - opacity) + filtered * opacity)));
    // Same fixed five-byte budget as the opaque Gaussian test: finite kernel,
    // downsample/resampling and RGBA8 intermediates, never fitted to results.
    if (!dx12_pixel(pixels, x, 64, {expected, 0, 0, expected}, 5))
      return dx12_fail(14, "premultiplied replacement differs from Gaussian step integral");
    const auto actual = pixels->pixel_rgba8(x, 64);
    if (!actual || (*actual)[0] != (*actual)[3] || (*actual)[0] <= previous)
      return dx12_fail(15, "filtered premultiplied red/alpha is inconsistent or nonmonotone");
    previous = (*actual)[0];
  }
  const auto right = pixels->pixel_rgba8(512 + unsigned(sigma / 4), 64);
  if (!right || (*right)[3] >= 240)
    return dx12_fail(16, "replacement did not reduce previously opaque right-side alpha");
  if (!dx12_pixel(pixels, 511, 4, {0, 0, 0, 0}) ||
      !dx12_pixel(pixels, 512, 4, {255, 0, 0, 255}))
    return dx12_fail(17, "alpha blur modified the original scene outside its mask");
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12BackdropPopupWindow window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "alpha step popup/watchdog unavailable");
  auto renderer = create_dx12_renderer(dx12_popup_descriptor(window.hwnd));
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  if (!(*renderer)->surface_composition().alpha_composited())
    return dx12_fail(3, "alpha step surface is not composited");
  if (!(*renderer)->resize({1024, 128}, {1})) return dx12_fail(4, "alpha step resize failed");
  // Four quality/downsample ranges, without using the production planner as
  // an oracle. Offset mask origin also catches scene sampling in local space.
  for (const float sigma : {4.0F, 12.0F, 32.0F, 64.0F}) {
    for (const float opacity : {1.0F, 0.5F}) {
      if (const int result = check_step(**renderer, sigma, opacity)) return result;
    }
  }
  return 0;
}
