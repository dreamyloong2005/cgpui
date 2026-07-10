#include "cgpui/platform/platform.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>

int main() {
  cgpui::test::WaylandTestCompositor compositor("child-ownership");
  if (!compositor.start()) {
    return 1;
  }
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  auto app = cgpui::create_platform_application();
  if (!app) {
    compositor.stop();
    return 2;
  }
  auto parent = (*app)->create_window(
      cgpui::WindowDescriptor{.title = "CGPUI Wayland Parent"},
      [](const cgpui::PlatformEvent&) {});
  if (!parent) {
    compositor.stop();
    return 3;
  }
  auto child = (*app)->create_child_window(
      cgpui::WindowDescriptor{.title = "CGPUI Wayland Child"},
      **parent,
      [](const cgpui::PlatformEvent&) {});
  const bool parented = child && compositor.wait_for_parent_requested();
  compositor.stop();
  return parented ? 0 : 4;
}
