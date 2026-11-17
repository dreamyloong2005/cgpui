#include "cgpui/platform/platform.hpp"
#include "wayland_dpi_awareness_internal.hpp"
#include "wayland_test_compositor.hpp"

#include <cstdlib>
#include <iostream>

namespace {

int fail(int code, const char* message) {
  std::cerr << "wayland dpi awareness: " << message << '\n';
  return code;
}

int check_state_mapping() {
  const auto fractional = cgpui::wayland_dpi_awareness_state(true);
  const auto integer = cgpui::wayland_dpi_awareness_state(false);
  if (fractional.awareness != cgpui::PlatformDpiAwareness::per_monitor_aware_v2 ||
      integer.awareness != cgpui::PlatformDpiAwareness::per_monitor_aware ||
      fractional.backend != "wayland" || integer.backend != "wayland" ||
      !fractional.declared_by_framework || fractional.already_set_by_process) {
    return fail(1, "awareness must follow the bound scale protocols");
  }
  return 0;
}

} // namespace

int main() {
  if (const int code = check_state_mapping(); code != 0) return code;
  // The test compositor advertises wp_fractional_scale_manager_v1 and
  // wp_viewporter, so the application binds both.
  cgpui::test::WaylandTestCompositor compositor("dpi-awareness");
  if (!compositor.start()) return fail(2, "test compositor did not start");
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  unsetenv("DISPLAY");
  auto app = cgpui::create_platform_application();
  if (!app) return fail(3, "wayland application was not created");
  const cgpui::PlatformDpiAwarenessState state = (*app)->dpi_awareness();
  compositor.stop();
  if (state.awareness != cgpui::PlatformDpiAwareness::per_monitor_aware_v2 ||
      state.backend != "wayland") {
    return fail(4, "fractional-scale compositor must report per-monitor v2");
  }
  return 0;
}
