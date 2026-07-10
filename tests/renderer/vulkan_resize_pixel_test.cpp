#include "vulkan_pixel_test_support.hpp"

#include <cstdio>

namespace {

bool capture_clear(
    cgpui::Renderer& renderer,
    cgpui::Color clear,
    const cgpui::RendererFramePixels*& pixels) {
  auto frame = renderer.begin_frame();
  if (!frame || !(*frame)->request_pixel_capture()) {
    return false;
  }
  (*frame)->clear(clear);
  if (!(*frame)->present()) {
    return false;
  }
  pixels = renderer.last_frame_pixels();
  return pixels != nullptr;
}

} // namespace

int main() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanResizePixelTestWindow", L"CGPUI Vulkan Resize Pixel Test");
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(),
      .scale = cgpui::DpiScale{1.0F},
  });
  if (!renderer) {
    return 1;
  }
  const cgpui::RendererFramePixels* captured = nullptr;
  if (!capture_clear(**renderer, {.r = 1.0F, .a = 1.0F}, captured)) {
    return 2;
  }
  const cgpui::RendererFramePixels first = *captured;
  const cgpui::Size resized = window.resize_client(96, 48);
  if (!(*renderer)->resize(resized, cgpui::DpiScale{1.0F})) {
    return 3;
  }
  if (!capture_clear(**renderer, {.g = 1.0F, .a = 1.0F}, captured)) {
    return 4;
  }
  const cgpui::RendererFramePixels second = *captured;
  const auto center = second.pixel_rgba8(second.width / 2, second.height / 2);
  const bool matches = first.width == 64 && first.height == 64 &&
                       second.width == 96 && second.height == 48 &&
                       cgpui_test::pixel_near(
                           second, second.width / 2, second.height / 2,
                           {0, 255, 0, 255});
  if (!matches) {
    std::fprintf(
        stderr,
        "resize captures first=%ux%u second=%ux%u center=%u,%u,%u,%u\n",
        first.width,
        first.height,
        second.width,
        second.height,
        center ? (*center)[0] : 0,
        center ? (*center)[1] : 0,
        center ? (*center)[2] : 0,
        center ? (*center)[3] : 0);
  }
  return matches ? 0 : 5;
}
