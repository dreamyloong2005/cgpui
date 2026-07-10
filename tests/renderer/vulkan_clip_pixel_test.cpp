#include "vulkan_pixel_test_support.hpp"

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanClipPixelTestWindow", L"CGPUI Vulkan Clip Pixel Test");
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) {
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {8.0F, 4.0F}, .size = {48.0F, 32.0F}},
            .color = {.r = 1.0F, .a = 1.0F},
            .clip_rect = cgpui::Rect{
                .origin = {8.0F, 4.0F}, .size = {24.0F, 16.0F}},
        });
      });
  return pixels &&
                 cgpui_test::pixel_near(*pixels, 20, 12, {255, 0, 0, 255}) &&
                 cgpui_test::pixel_near(*pixels, 44, 12, {0, 0, 0, 255}) &&
                 cgpui_test::pixel_near(*pixels, 20, 28, {0, 0, 0, 255})
             ? 0
             : 1;
}
