#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <cstdlib>
#include <string_view>

int main() {
  const char* display = std::getenv("WAYLAND_DISPLAY");
  if (display == nullptr || std::string_view(display).empty()) {
    return 0;
  }
  auto app = cgpui::create_platform_application();
  if (!app) {
    return 1;
  }
  auto window = (*app)->create_window(
      {.title = "CGPUI Wayland Pixel Capture", .size = {64.0F, 64.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) {
    return 2;
  }
  const cgpui::WindowState state = (*window)->state();
  auto renderer = cgpui::create_renderer({
      .native_surface = (*window)->native_surface(),
      .framebuffer_size = state.framebuffer_size,
      .scale = state.scale,
  });
  if (!renderer) {
    return 3;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame || !(*frame)->request_pixel_capture()) {
    return 4;
  }
  (*frame)->clear({.r = 1.0F, .b = 1.0F, .a = 1.0F});
  if (!(*frame)->present()) {
    return 5;
  }
  const cgpui::RendererFramePixels* pixels = (*renderer)->last_frame_pixels();
  const auto center = pixels != nullptr
                          ? pixels->pixel_rgba8(pixels->width / 2,
                                                pixels->height / 2)
                          : std::nullopt;
  (*window)->request_close();
  return center && (*center)[0] == 255 && (*center)[1] == 0 &&
                 (*center)[2] == 255 && (*center)[3] == 255
             ? 0
             : 6;
}
