#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <variant>

int main() {
  const char* wayland_display = std::getenv("WAYLAND_DISPLAY");
  if (wayland_display == nullptr || std::string_view(wayland_display).empty()) {
    std::cout << "WAYLAND_DISPLAY is not set; skipping Wayland Vulkan smoke\n";
    return 0;
  }

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 3;
  }

  bool close_requested = false;
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Vulkan Surface Test",
          .size = cgpui::Size{160.0F, 120.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close_requested = true;
          (*app)->quit();
        }
      });
  if (!window) {
    std::cerr << window.error().message << '\n';
    return 4;
  }

  const cgpui::NativeSurfaceHandle native_surface = (*window)->native_surface();
  if (!std::holds_alternative<cgpui::WaylandSurfaceHandle>(native_surface)) {
    return 5;
  }

  const cgpui::WindowState window_state = (*window)->state();
  auto renderer = cgpui::create_renderer(cgpui::RenderSurfaceDescriptor{
      .native_surface = native_surface,
      .framebuffer_size = window_state.framebuffer_size,
      .scale = window_state.scale});
  if (!renderer) {
    std::cerr << renderer.error().message << '\n';
    return 6;
  }

  auto frame = (*renderer)->begin_frame();
  if (!frame) {
    std::cerr << frame.error().message << '\n';
    return 7;
  }

  (*frame)->clear(cgpui::Color{.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  if (auto presented = (*frame)->present(); !presented) {
    std::cerr << presented.error().message << '\n';
    return 8;
  }

  (*window)->request_close();
  (void)(*app)->run();

  return close_requested ? 0 : 9;
}
