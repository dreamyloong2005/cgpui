#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"

#include <cstdlib>
#include <iostream>
#include <string_view>
#include <variant>

int main() {
  const char* display = std::getenv("DISPLAY");
  if (display == nullptr || std::string_view(display).empty()) return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");

  auto app = cgpui::create_platform_application();
  if (!app) {
    std::cerr << app.error().message << '\n';
    return 1;
  }
  cgpui::PlatformWindow* observed = nullptr;
  bool close_requested = false;
  auto window = (*app)->create_window(
      {.title = "CGPUI X11 Vulkan Surface", .size = {160.0F, 120.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (std::holds_alternative<cgpui::WindowCloseRequested>(event)) {
          close_requested = true;
          if (observed != nullptr) {
            observed->resolve_close_request(
                cgpui::PlatformWindowCloseResolution::accept);
          }
          (*app)->quit();
        }
      });
  if (!window) {
    std::cerr << window.error().message << '\n';
    return 2;
  }
  observed = window->get();
  const cgpui::WindowState state = (*window)->state();
  auto renderer = cgpui::create_renderer({
      .native_surface = (*window)->native_surface(),
      .framebuffer_size = state.framebuffer_size,
      .scale = state.scale,
  });
  if (!renderer) {
    std::cerr << renderer.error().message << '\n';
    return 3;
  }
  auto frame = (*renderer)->begin_frame();
  if (!frame) return 4;
  (*frame)->clear({.r = 0.08F, .g = 0.09F, .b = 0.10F, .a = 1.0F});
  if (!(*frame)->present()) return 5;

  (*window)->request_close();
  const int run_result = (*app)->run();
  return run_result == 0 && close_requested ? 0 : 6;
}
