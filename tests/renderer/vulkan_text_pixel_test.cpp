#include "vulkan_pixel_test_support.hpp"

#include <string_view>

namespace {

cgpui::TextDraw text_draw(std::string_view content) {
  const cgpui::FontDescriptor font{.family = "Inter"};
  return cgpui::TextDraw{
      .bounds = {.origin = {8.0F, 8.0F}, .size = {48.0F, 24.0F}},
      .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 1.0F},
      .font = font,
      .content = std::string(content),
      .byte_length = content.size(),
      .font_size = 20.0F,
      .device_font_size = 20.0F,
      .glyphs = cgpui::text_glyph_paint_metadata(
          cgpui::shape_text(content, font, 20.0F), {8.0F, 8.0F}),
  };
}

} // namespace

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanTextPixelTestWindow", L"CGPUI Vulkan Text Pixel Test");
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.a = 1.0F},
      [](cgpui::RenderFrame& frame) { frame.draw_text(text_draw("A")); });
  if (!pixels || !pixels->valid() ||
      !cgpui_test::pixel_near(*pixels, 1, 1, {0, 0, 0, 255})) {
    return 1;
  }
  std::size_t top_foreground_count = 0;
  std::size_t bottom_foreground_count = 0;
  for (std::uint32_t y = 0; y < pixels->height; ++y) {
    for (std::uint32_t x = 0; x < pixels->width; ++x) {
      const auto pixel = pixels->pixel_rgba8(x, y);
      if (pixel && (*pixel)[0] > 240 && (*pixel)[1] > 240 &&
          (*pixel)[2] > 240) {
        y < pixels->height / 2 ? ++top_foreground_count
                               : ++bottom_foreground_count;
      }
    }
  }
  return top_foreground_count >= 20 && bottom_foreground_count == 0 ? 0 : 2;
}
