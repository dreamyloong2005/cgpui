#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>

int main() {
  cgpui::test::WaylandTestCompositor compositor("position");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);

  auto app = cgpui::create_platform_application();
  if (!app) {
    compositor.stop();
    return 2;
  }
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Window Position Test",
          .size = cgpui::Size{420.0F, 280.0F},
          .position = cgpui::Point{160.0F, 120.0F}},
      [](const cgpui::PlatformEvent&) {});
  if (!window) {
    compositor.stop();
    return 3;
  }
  const auto state = (*window)->position_state();
  const bool request_result =
      (*window)->request_position(cgpui::Point{240.0F, 180.0F});
  compositor.stop();
  if (state.supported || state.position.has_value() || request_result) {
    return 4;
  }
  return 0;
}
