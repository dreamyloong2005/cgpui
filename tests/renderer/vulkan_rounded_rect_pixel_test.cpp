#include "vulkan_pixel_test_support.hpp"

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanRoundedRectPixelTestWindow",
      L"CGPUI Vulkan Rounded Rect Pixel Test");
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) {
        frame.draw_rounded_rect(cgpui::RoundedRectDraw{
            .rect = {.origin = {8.0F, 4.0F}, .size = {40.0F, 24.0F}},
            .color = {.r = 1.0F, .a = 1.0F},
            .radius = cgpui::BorderRadii::all(8.0F),
        });
      });
  return pixels &&
                 cgpui_test::pixel_near(*pixels, 28, 16, {255, 0, 0, 255}) &&
                 cgpui_test::pixel_near(*pixels, 9, 5, {0, 0, 0, 255}, 4)
             ? 0
             : 1;
}
