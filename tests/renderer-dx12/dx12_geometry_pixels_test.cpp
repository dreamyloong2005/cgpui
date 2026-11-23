#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "native window failed");
  auto descriptor = window.descriptor();
  descriptor.scale = {1.5F};
  auto renderer = create_dx12_renderer(descriptor);
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(3, frame.error().message.c_str());
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->draw_rect({.rect = {{0, 0}, {20, 10}}, .color = {1, 0, 0, 1}});
  (*frame)->draw_rect({.rect = {{20, 20}, {20, 20}}, .color = {0, 1, 0, 1},
                      .clip_rect = Rect{{20, 20}, {10, 10}}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) {
    return dx12_fail(4, "scaled capture failed");
  }
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!dx12_pixel(pixels, 29, 14, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 31, 7, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 31, 31, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 44, 44, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 46, 40, {0, 0, 0, 255})) {
    return dx12_fail(5, "DPI scale or clip differs");
  }
  frame->reset();
  if (!(*renderer)->resize({64, 64}, {1})) return dx12_fail(6, "scale reset failed");
  frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(7, "second begin failed");
  (*frame)->clear({0, 0, 0, 1});
  const PaintMetadata composed{
      .opacity = 0.5F, .transform = AffineTransform::translation(24, 8)};
  (*frame)->draw_rect({
      .rect = {{4, 4}, {16, 16}}, .color = {1, 0, 0, 1},
      .composition_stack = renderer_composition_stack_record(std::span{&composed, 1}),
      .metadata = composed});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) {
    return dx12_fail(8, "composition capture failed");
  }
  pixels = (*renderer)->last_frame_pixels();
  const unsigned char half = pixels && pixels->encoding == RendererFramePixelEncoding::srgb
                                 ? 188 : 128;
  if (!dx12_pixel(pixels, 12, 12, {0, 0, 0, 255}) ||
      !dx12_pixel(pixels, 36, 20, {half, 0, 0, 255}) ||
      !dx12_pixel(pixels, 36, 36, {0, 0, 0, 255})) {
    return dx12_fail(9, "composition opacity/translation applied incorrectly");
  }
  frame->reset();
  frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(10, "third begin failed");
  (*frame)->clear({0, 0, 0, 1});
  (*frame)->draw_rounded_rect({.rect = {{8, 4}, {40, 24}},
      .color = {1, 0, 0, 1}, .radius = BorderRadii::all(8)});
  // Interleave solid/rounded calls; batching must preserve submission order.
  (*frame)->draw_rect({.rect = {{24, 8}, {8, 12}}, .color = {0, 1, 0, 1}});
  (*frame)->draw_rounded_rect({.rect = {{26, 10}, {4, 4}},
      .color = {0, 0, 1, 1}, .radius = BorderRadii::all(1)});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present()) {
    return dx12_fail(11, "rounded capture failed");
  }
  pixels = (*renderer)->last_frame_pixels();
  if (!dx12_pixel(pixels, 16, 16, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 9, 5, {0, 0, 0, 255}, 4) ||
      !dx12_pixel(pixels, 25, 16, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 28, 12, {0, 0, 255, 255})) {
    return dx12_fail(12, "rounded geometry or draw ordering differs");
  }
  return 0;
}
