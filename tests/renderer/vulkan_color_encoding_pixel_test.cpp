#include "vulkan_pixel_test_support.hpp"

#include <cstdio>

// Color components are sRGB-encoded values; the presented bytes must equal
// them. With a B8G8R8A8_SRGB swapchain rgb(32,34,38) became rgb(99,102,108)
// and the runtime clear 0.08/0.09/0.10 became rgb(80,84,89).
int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanColorEncodingPixelTestWindow",
      L"CGPUI Vulkan Color Encoding Pixel Test");
  constexpr auto channel = [](int value) {
    return static_cast<float>(value) / 255.0F;
  };
  auto pixels = cgpui_test::capture_pixels(
      window,
      {.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F},
      [&](cgpui::RenderFrame& frame) {
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {0.0F, 0.0F}, .size = {64.0F, 32.0F}},
            .color = {.r = channel(32), .g = channel(34), .b = channel(38), .a = 1.0F},
        });
        frame.draw_rect(cgpui::SolidRect{
            .rect = {.origin = {0.0F, 32.0F}, .size = {32.0F, 32.0F}},
            .color = {.r = 1.0F, .g = 1.0F, .b = 1.0F, .a = 0.5F},
        });
      });
  if (!pixels) return 1;
  if (pixels->encoding != cgpui::RendererFramePixelEncoding::linear) {
    std::fprintf(stderr, "swapchain encoding is not UNORM\n");
    return 2;
  }
  const struct {
    std::uint32_t x;
    std::uint32_t y;
    std::array<std::uint8_t, 4> rgba;
  } probes[] = {
      {16, 16, {32, 34, 38, 255}},
      {48, 48, {20, 23, 26, 255}},
      // 50% white over the clear, blended in sRGB-encoded space.
      {16, 48, {138, 139, 140, 255}},
  };
  int code = 2;
  for (const auto& probe : probes) {
    ++code;
    if (!cgpui_test::pixel_near(*pixels, probe.x, probe.y, probe.rgba, 2)) {
      const auto actual = pixels->pixel_rgba8(probe.x, probe.y);
      std::fprintf(stderr, "probe %u,%u got %u,%u,%u\n", probe.x, probe.y,
                   actual ? (*actual)[0] : 0, actual ? (*actual)[1] : 0,
                   actual ? (*actual)[2] : 0);
      return code;
    }
  }
  return 0;
}
