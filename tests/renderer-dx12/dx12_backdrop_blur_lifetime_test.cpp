#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int effects_frame(Renderer& renderer, Size size) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(10, "lifecycle effects begin failed");
  (*frame)->clear({0, 0, 1, 1});
  // Three differently sized regions and sigmas exercise independent transient
  // targets. Uniform colors and opaque tints give exact independent samples.
  (*frame)->draw_backdrop_blur({.rect = {{0, 0}, size}, .blur_radius = 2});
  (*frame)->draw_backdrop_blur({.rect = {{4, 4}, {17, 19}},
      .blur_radius = 12, .tint = Color{1, 0, 0, 1}});
  (*frame)->draw_backdrop_blur({.rect = {{35, 24}, {21, 17}},
      .blur_radius = 32, .tint = Color{0, 1, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "lifecycle effects capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || pixels->width != unsigned(size.width) || pixels->height != unsigned(size.height) ||
      !dx12_pixel(pixels, 2, 2, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 12, 12, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 44, 32, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, pixels->width - 1, pixels->height - 1, {0, 0, 255, 255}))
    return dx12_fail(13, "odd-size transient targets or blur viewport leaked");
  return 0;
}

int plain_frame(Renderer& renderer, Size size) {
  auto frame = cgpui_test::wait_for_test_frame(renderer);
  if (!frame) return dx12_fail(20, "plain frame after blur unavailable");
  (*frame)->clear({0, 1, 1, 1});
  (*frame)->draw_rect({.rect = {{4, 4}, {17, 19}}, .color = {1, 0, 1, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(21, "plain capture refused");
  if (auto result = (*frame)->present(); !result)
    return dx12_fail(22, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  if (!pixels || pixels->width != unsigned(size.width) || pixels->height != unsigned(size.height) ||
      !dx12_pixel(pixels, 12, 12, {255, 0, 255, 255}) ||
      !dx12_pixel(pixels, 44, 32, {0, 255, 255, 255}) ||
      !dx12_pixel(pixels, pixels->width - 1, pixels->height - 1, {0, 255, 255, 255}))
    return dx12_fail(23, "plain frame inherited stale blur targets or pixels");
  return 0;
}
} // namespace

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "lifecycle window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto abandoned = cgpui_test::wait_for_test_frame(**renderer);
  if (!abandoned) return dx12_fail(3, "abandoned blur begin failed");
  (*abandoned)->clear({1, 0, 0, 1});
  (*abandoned)->draw_backdrop_blur({.rect = {{0, 0}, {64, 64}}, .blur_radius = 12});
  if ((*renderer)->begin_frame() || (*renderer)->resize({65, 49}, {1}))
    return dx12_fail(4, "active backdrop frame lost exclusive ownership");
  abandoned->reset();
  for (const Size size : {Size{65, 49}, Size{131, 97}, Size{67, 51}}) {
    if (!(*renderer)->resize(size, {1})) return dx12_fail(5, "odd resize after blur failed");
    if (const int result = effects_frame(**renderer, size)) return result;
    if (const int result = plain_frame(**renderer, size)) return result;
  }
  // A second live renderer must not borrow the first renderer's effect targets.
  Dx12Window other_window;
  if (!other_window.hwnd) return dx12_fail(30, "second lifecycle window unavailable");
  auto other = create_dx12_renderer(other_window.descriptor());
  if (!other) return dx12_fail(31, other.error().message.c_str());
  if (const int result = effects_frame(**other, {64, 64})) return result;
  if (!dx12_pixel((*renderer)->last_frame_pixels(), 44, 32, {0, 255, 255, 255}))
    return dx12_fail(32, "second renderer changed first capture");
  other->reset();
  if (const int result = effects_frame(**renderer, {67, 51})) return result;
  auto orphan = cgpui_test::wait_for_test_frame(**renderer);
  if (!orphan) return dx12_fail(33, "orphan backdrop begin failed");
  (*orphan)->draw_backdrop_blur({.rect = {{4, 4}, {48, 40}}, .blur_radius = 16});
  renderer->reset();
  const auto capture = (*orphan)->request_pixel_capture();
  const auto result = (*orphan)->present();
  if (!capture || !result || (*orphan)->present())
    return dx12_fail(34, "healthy orphan backdrop frame must finish exactly once with HWND alive");
  orphan->reset();
  return 0;
}
