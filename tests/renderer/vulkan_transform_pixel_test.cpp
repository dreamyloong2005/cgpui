#include "vulkan_pixel_test_support.hpp"

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanTransformPixelTestWindow",
      L"CGPUI Vulkan Transform Pixel Test");
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) {
        const cgpui::PaintMetadata metadata{
            .transform = cgpui::AffineTransform::translation(24.0F, 8.0F),
        };
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {4.0F, 4.0F}, .size = {16.0F, 16.0F}},
            .color = {.g = 1.0F, .a = 1.0F},
            .composition_stack = cgpui::renderer_composition_stack_record(
                std::span<const cgpui::PaintMetadata>(&metadata, 1)),
            .metadata = metadata,
        });
      });
  return pixels &&
                 cgpui_test::pixel_near(*pixels, 12, 12, {0, 0, 0, 255}) &&
                 cgpui_test::pixel_near(*pixels, 36, 20, {0, 255, 0, 255}) &&
                 cgpui_test::pixel_near(*pixels, 36, 36, {0, 0, 0, 255})
             ? 0
             : 1;
}
