#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "shadow window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(3, "shadow begin failed");
  (*frame)->clear({1, 1, 1, 1});
  (*frame)->draw_box_shadow({.rect = {{16, 16}, {16, 16}},
      .shadow = {.color = {0, 0, 0, 1}, .offset = {8, 4}, .blur_radius = 0}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(4, "shadow capture refused");
  const auto result = (*frame)->present();
  if (!result) return dx12_fail(5, result.error().message.c_str());
  // Literal hard-shadow oracle: translated shape [24,40)x[20,36), minus
  // the original box [16,32)x[16,32). Samples stay away from AA edges.
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!dx12_pixel(pixels, 36, 24, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 28, 34, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 28, 24, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 20, 20, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 12, 24, {255, 255, 255, 255}) ||
      !dx12_pixel(pixels, 44, 24, {255, 255, 255, 255}))
    return dx12_fail(6, "hard shadow offset or box clip-out differs");
  return 0;
}
