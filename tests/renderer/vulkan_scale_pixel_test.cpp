#include "vulkan_pixel_test_support.hpp"

#include <cstdio>

// At DpiScale 1.5 a logical rect covers 1.5x as many device pixels, and
// clip rects scale with it. Before the fix logical pixels were drawn 1:1.
int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanScalePixelTestWindow", L"CGPUI Vulkan Scale Pixel Test");
  auto pixels = cgpui_test::capture_pixels_at_scale(
      window,
      cgpui::DpiScale{1.5F},
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) {
        // Device rect 0,0 30x15.
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {0.0F, 0.0F}, .size = {20.0F, 10.0F}},
            .color = {.r = 1.0F, .a = 1.0F},
        });
        // Device clip 30,30 15x15.
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {20.0F, 20.0F}, .size = {20.0F, 20.0F}},
            .color = {.g = 1.0F, .a = 1.0F},
            .clip_rect = cgpui::Rect{
                .origin = {20.0F, 20.0F}, .size = {10.0F, 10.0F}},
        });
      });
  if (!pixels) return 1;
  const struct {
    std::uint32_t x;
    std::uint32_t y;
    std::array<std::uint8_t, 4> rgba;
  } probes[] = {
      {29, 14, {255, 0, 0, 255}},
      {31, 7, {0, 0, 0, 255}},
      {15, 16, {0, 0, 0, 255}},
      {31, 31, {0, 255, 0, 255}},
      {44, 44, {0, 255, 0, 255}},
      {46, 40, {0, 0, 0, 255}},
      {40, 46, {0, 0, 0, 255}},
  };
  int code = 0;
  for (const auto& probe : probes) {
    ++code;
    if (!cgpui_test::pixel_near(*pixels, probe.x, probe.y, probe.rgba)) {
      const auto actual = pixels->pixel_rgba8(probe.x, probe.y);
      std::fprintf(stderr, "probe %u,%u got %u,%u,%u\n", probe.x, probe.y,
                   actual ? (*actual)[0] : 0, actual ? (*actual)[1] : 0,
                   actual ? (*actual)[2] : 0);
      return 10 + code;
    }
  }
  return 0;
}
