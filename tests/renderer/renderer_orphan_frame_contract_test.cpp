#include "renderer_pixel_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  RendererPixelTestWindow window(L"CGPUIRendererOrphanFrameContract", L"Renderer orphan frame contract");
  if (!window.valid()) return 1;
  auto renderer = create_pixel_test_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2; // Shared helper requires the requested actual backend, never fallback.
  {
    auto first = wait_for_test_frame(**renderer);
    if (!first) return 3;
    (*first)->clear({1, 0, 0, 1});
    if (!(*first)->request_pixel_capture() || !(*first)->present()) return 4;
    const auto* pixels = (*renderer)->last_frame_pixels();
    if (!pixels || !pixels->valid() || !pixel_near(*pixels, 8, 8, {255, 0, 0, 255})) return 5;
  }
  auto frame = wait_for_test_frame(**renderer);
  if (!frame) return 6;
  renderer->reset(); // HWND remains alive until after the independent frame is destroyed.
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{4, 4}, {16, 16}}, .color = {0, 1, 0, 1}});
  const auto capture = (*frame)->request_pixel_capture();
  const auto presented = (*frame)->present(); // Always execute the real original orphan-Present oracle.
  if (!pixel_test_ok(presented, "orphan present after facade destruction")) return 40;
  if (!pixel_test_ok(capture, "orphan capture request after facade destruction")) return 7;
  frame->reset();
  return 0; // No dead Renderer getter: orphan pixels are not asserted by this shared hidden test.
}
