#include "vulkan_pixel_test_support.hpp"

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanOpacityPixelTestWindow",
      L"CGPUI Vulkan Opacity Pixel Test");
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) {
        const cgpui::PaintMetadata metadata{.opacity = 0.5F};
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {8.0F, 4.0F}, .size = {40.0F, 20.0F}},
            .color = {.r = 1.0F, .a = 1.0F},
            .composition_stack = cgpui::renderer_composition_stack_record(
                std::span<const cgpui::PaintMetadata>(&metadata, 1)),
            .metadata = metadata,
        });
      });
  if (!pixels) {
    return 1;
  }
  const std::uint8_t half =
      cgpui_test::encoded_channel(0.5F, pixels->encoding);
  return cgpui_test::pixel_near(*pixels, 28, 12, {half, 0, 0, 255}, 3)
             ? 0
             : 2;
}
