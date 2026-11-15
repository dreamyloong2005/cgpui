#include "vulkan_pixel_test_support.hpp"
#include "vulkan_text_pixel_shape_support.hpp"

#include <iostream>
#include <string_view>

namespace {

cgpui::TextDraw text_draw(std::string_view content) {
  const cgpui::FontDescriptor font{.family = "Segoe UI"};
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
  std::vector<std::vector<std::uint8_t>> masks;
  for (const auto* content : {"A", "B", "\xE4\xB8\xAD", "\xE6\x96\x87", " "}) {
    auto pixels = cgpui_test::capture_pixels(window, {.a = 1.0F},
        [&](cgpui::RenderFrame& frame) { frame.draw_text(text_draw(content)); });
    if (!pixels || !pixels->valid() ||
        !cgpui_test::pixel_near(*pixels, 1, 1, {0, 0, 0, 255})) return 1;
    const auto ink = cgpui_test::text_ink_shape(*pixels);
    if (std::string_view(content) == " ") {
      if (ink.pixels != 0) return 2;
    } else if (!ink.shaped() || ink.bottom > pixels->height / 2) {
      std::cerr << "Glyph has no recognizable silhouette: ink=" << ink.pixels << '\n';
      return 3;
    }
    masks.push_back(std::move(pixels->rgba8));
  }
  if (masks[0] == masks[1] || masks[2] == masks[3]) return 4;
  std::cout << "Vulkan pixel capture passed: distinct Latin/CJK silhouettes and transparent whitespace.\n";
  return 0;
}
