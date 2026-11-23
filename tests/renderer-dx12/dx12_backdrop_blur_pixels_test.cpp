#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "backdrop window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(3, "backdrop begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_rect({.rect = {{0, 0}, {32, 64}}, .color = {0, 0, 0, 1}});
  (*frame)->draw_backdrop_blur({.rect = {{8, 8}, {48, 48}}, .blur_radius = 4});
  (*frame)->draw_rect({.rect = {{30, 28}, {4, 8}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(4, "backdrop capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(5, result.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || !pixels->valid()) return dx12_fail(6, "backdrop capture unavailable");
  const auto left = pixels->pixel_rgba8(28, 20);
  const auto right = pixels->pixel_rgba8(35, 20);
  if (!left || !right) return dx12_fail(7, "backdrop edge samples unavailable");
  // A Gaussian-blurred black/white step is a monotone gray transition. At
  // +/-3.5 px and sigma 4 its ideal values are about 49/206. These deliberately
  // broad tracer bands prove spatial mixing without prescribing a GPU kernel;
  // a no-op or tint-only fallback (no tint requested) leaves exactly 0/255.
  for (unsigned channel = 0; channel < 3; ++channel) {
    if ((*left)[channel] < 24 || (*left)[channel] > 96 ||
        (*right)[channel] < 159 || (*right)[channel] > 231)
      return dx12_fail(8, "backdrop did not smooth previously drawn edge");
  }
  if ((*left)[3] != 255 || (*right)[3] != 255 ||
      !dx12_pixel(pixels, 28, 4, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 35, 4, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 31, 30, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 32, 34, {255, 0, 0, 255}))
    return dx12_fail(9, "backdrop escaped panel or reordered later foreground");
  return 0;
}
