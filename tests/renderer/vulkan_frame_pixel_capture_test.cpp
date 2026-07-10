#include "vulkan_pixel_test_support.hpp"

namespace {

int test_captures_presented_clear_color() {
  cgpui_test::VulkanPixelTestWindow window(
      L"CGPUIVulkanFramePixelCaptureTestWindow",
      L"CGPUI Vulkan Frame Pixel Capture Test");
  if (!window.valid()) {
    return 1;
  }
  const cgpui::Size framebuffer_size = window.framebuffer_size();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = window.surface(),
      .framebuffer_size = framebuffer_size,
      .scale = cgpui::DpiScale{1.0F},
  });
  if (!renderer) {
    return 2;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    return 3;
  }
  if (auto requested = (*frame)->request_pixel_capture(); !requested) {
    return 4;
  }
  (*frame)->clear({.r = 1.0F, .g = 0.0F, .b = 0.0F, .a = 1.0F});
  if (auto presented = (*frame)->present(); !presented) {
    return 5;
  }
  const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
  if (pixels == nullptr || !pixels->valid() ||
      pixels->width != static_cast<std::uint32_t>(framebuffer_size.width) ||
      pixels->height != static_cast<std::uint32_t>(framebuffer_size.height)) {
    return 6;
  }
  return cgpui_test::pixel_near(
             *pixels,
             pixels->width / 2,
             pixels->height / 2,
             {255, 0, 0, 255})
             ? 0
             : 7;
}

} // namespace

int main() {
  return test_captures_presented_clear_color();
}
