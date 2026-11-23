#include "dx12_popup_backdrop_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int rounded_mask(Renderer& renderer) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(10, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 0});
  (*frame)->draw_rect({.rect = {{0, 0}, {64, 64}}, .color = {1, 0, 0, 0.5F}});
  (*frame)->draw_backdrop_blur({.rect = {{10.5F, 10.5F}, {40, 40}},
      .radius = BorderRadii::all(10), .blur_radius = 4, .tint = Color{0, 0, 1, 0.5F},
      .clip_rect = Rect{{0, 0}, {40, 64}}, .metadata = {.opacity = 0.5F}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(11, "rounded alpha capture refused");
  if (const auto result = (*frame)->present(); !result)
    return dx12_fail(12, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  // Uniform half-red stays uniform through blur. Full coverage gives
  // (.375,0,.25,.625); coverage .5 gives (.4375,0,.125,.5625).
  // The corner center is (20.5,20.5). Pixel (14,12) is exactly radius 10
  // away (6-8-10 triangle), hence signed distance zero and half coverage.
  if (!dx12_pixel(pixels, 24, 24, {96, 0, 64, 160}, 2) ||
      !dx12_pixel(pixels, 10, 24, {112, 0, 32, 144}, 2) ||
      !dx12_pixel(pixels, 24, 10, {112, 0, 32, 144}, 2) ||
      !dx12_pixel(pixels, 14, 12, {112, 0, 32, 144}, 2) ||
      !dx12_pixel(pixels, 10, 10, {128, 0, 0, 128}) ||
      !dx12_pixel(pixels, 44, 24, {128, 0, 0, 128}) ||
      !dx12_pixel(pixels, 4, 24, {128, 0, 0, 128}))
    return dx12_fail(13, "rounded fractional alpha mask or clip differs");
  return 0;
}

int overlapping_layers(Renderer& renderer) {
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(20, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 0});
  (*frame)->draw_rect({.rect = {{0, 0}, {64, 64}}, .color = {1, 0, 0, 0.5F}});
  (*frame)->draw_backdrop_blur({.rect = {{0, 0}, {40, 64}}, .blur_radius = 0,
      .tint = Color{0, 0, 1, 0.5F}, .metadata = {.opacity = 0.5F}});
  (*frame)->draw_rect({.rect = {{32, 0}, {32, 64}}, .color = {0, 1, 0, 0.5F}});
  (*frame)->draw_backdrop_blur({.rect = {{24, 0}, {40, 64}}, .blur_radius = 0,
      .tint = Color{1, 1, 1, 0.5F}, .metadata = {.opacity = 0.5F}});
  (*frame)->draw_rect({.rect = {{4, 44}, {56, 8}}, .color = {1, 1, 0, 0.5F}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(21, "ordered alpha capture refused");
  if (const auto result = (*frame)->present(); !result)
    return dx12_fail(22, result.error().message.c_str());
  const auto* pixels = renderer.last_frame_pixels();
  // First effect gives (96,0,64,160). The intervening half-green then gives
  // (48,128,32,208) in the overlap and (64,128,0,192) to its right. The
  // half-opacity/half-white second effect contributes .25 white + .75 scene,
  // so it must see both the first effect and the intervening primitive.
  if (!dx12_pixel(pixels, 12, 24, {96, 0, 64, 160}, 2) ||
      !dx12_pixel(pixels, 28, 24, {136, 64, 112, 184}, 2) ||
      !dx12_pixel(pixels, 36, 24, {100, 160, 88, 220}, 2) ||
      !dx12_pixel(pixels, 52, 24, {112, 160, 64, 208}, 2) ||
      !dx12_pixel(pixels, 12, 48, {176, 128, 32, 208}, 2) ||
      !dx12_pixel(pixels, 36, 48, {178, 208, 44, 238}, 2) ||
      !dx12_pixel(pixels, 52, 48, {184, 208, 32, 232}, 2))
    return dx12_fail(23, "alpha backdrop snapshot coordinates/order or later foreground differs");
  return 0;
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12BackdropPopupWindow window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "alpha layers popup/watchdog unavailable");
  auto renderer = create_dx12_renderer(dx12_popup_descriptor(window.hwnd));
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  if (!(*renderer)->surface_composition().alpha_composited())
    return dx12_fail(3, "alpha layers surface is not composited");
  if (const int result = rounded_mask(**renderer)) return result;
  return overlapping_layers(**renderer);
}
