#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>

int main() {
  cgpui::test::WaylandTestCompositor compositor("chrome");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) {
    compositor.stop();
    return 2;
  }
  const cgpui::WindowChromeOptions frameless{
      .titlebar_visible = false,
      .decorations = false,
      .resizable = false,
      .transparent_background = true};
  auto window = (*app)->create_window(
      cgpui::WindowDescriptor{
          .title = "CGPUI Wayland Window Chrome Test",
          .size = {420.0F, 280.0F},
          .chrome = frameless},
      [](const cgpui::PlatformEvent&) {});
  if (!window) {
    compositor.stop();
    return 3;
  }
  const auto frameless_state = (*window)->apply_window_chrome(frameless);
  if (!frameless_state.supported ||
      !frameless_state.decoration_control_supported ||
      !frameless_state.transparency_supported ||
      frameless_state.applied.decorations ||
      !frameless_state.applied.transparent_background ||
      !compositor.wait_for_client_side_decoration_requested()) {
    compositor.stop();
    return 4;
  }
  const auto decorated_state = (*window)->apply_window_chrome({});
  const bool decorated = decorated_state.supported &&
      decorated_state.applied.decorations &&
      decorated_state.applied.titlebar_visible &&
      compositor.wait_for_server_side_decoration_requested();
  compositor.stop();
  return decorated ? 0 : 5;
}
