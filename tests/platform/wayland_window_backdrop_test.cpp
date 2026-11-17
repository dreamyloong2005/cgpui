#include "cgpui/platform/platform.hpp"
#include "wayland_backdrop_internal.hpp"
#include "wayland_test_background_effect.hpp"
#include "wayland_test_compositor.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>

namespace {

using cgpui::WaylandBlurProtocol;
using cgpui::WindowBackdrop;

int fail(int code, const char* message) {
  std::cerr << "wayland window backdrop: " << message << '\n';
  return code;
}

int check_plan() {
  const auto absent = cgpui::wayland_plan_window_backdrop(
      WindowBackdrop::mica, true, WaylandBlurProtocol::none);
  if (absent.state.supported || absent.blur || absent.state.reason.empty()) {
    return fail(1, "no blur protocol must be unsupported with a reason");
  }
  const auto opaque = cgpui::wayland_plan_window_backdrop(
      WindowBackdrop::blur, false, WaylandBlurProtocol::kde_blur);
  if (!opaque.state.supported || opaque.blur || opaque.state.reason.empty()) {
    return fail(2, "an opaque surface must not claim blur");
  }
  const auto mapped = cgpui::wayland_plan_window_backdrop(
      WindowBackdrop::acrylic, true, WaylandBlurProtocol::ext_background_effect);
  if (!mapped.blur || mapped.state.applied != WindowBackdrop::blur ||
      mapped.state.mechanism != "ext_background_effect_v1") {
    return fail(3, "acrylic must map to compositor blur");
  }
  cgpui::WaylandBackdropGlobals globals;
  if (cgpui::wayland_blur_protocol(globals) != WaylandBlurProtocol::none) {
    return fail(4, "empty globals have no protocol");
  }
  return 0;
}

std::unique_ptr<cgpui::PlatformApplication> connect(
    cgpui::test::WaylandTestCompositor& compositor) {
  setenv("WAYLAND_DISPLAY", compositor.socket_name().c_str(), 1);
  unsetenv("DISPLAY");
  auto app = cgpui::create_platform_application();
  return app ? std::move(*app) : nullptr;
}

cgpui::WindowDescriptor mica_descriptor() {
  cgpui::WindowDescriptor descriptor;
  descriptor.size = cgpui::Size{.width = 200.0F, .height = 120.0F};
  descriptor.backdrop = WindowBackdrop::mica;
  return descriptor;
}

} // namespace

int main() {
  if (const int code = check_plan(); code != 0) return code;
  // Keep the portal lookup off the real session bus.
  setenv("DBUS_SESSION_BUS_ADDRESS", "unix:path=/nonexistent/cgpui-bus", 1);
  {
    cgpui::test::WaylandTestCompositor compositor("backdrop-absent");
    if (!compositor.start()) return fail(10, "test compositor did not start");
    auto app = connect(compositor);
    if (!app) return fail(11, "wayland application was not created");
    if (app->system_appearance().transparency_effects_enabled) {
      return fail(12, "no blur protocol means transparency is off");
    }
    auto window = app->create_window(mica_descriptor(), [](const auto&) {});
    if (!window) return fail(13, "window was not created");
    const auto state = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
    if (state.supported || state.applied != WindowBackdrop::none ||
        state.backend != "wayland" || state.reason.empty()) {
      return fail(14, "absent protocol must report unsupported");
    }
    window->reset();
    app.reset();
    compositor.stop();
  }
  cgpui::test::WaylandFakeBackgroundEffect effect;
  cgpui::test::WaylandTestCompositor compositor("backdrop-present");
  compositor.add_global_installer(
      [&effect](wl_display* display) { effect.install(display); });
  if (!compositor.start()) return fail(20, "test compositor did not start");
  auto app = connect(compositor);
  if (!app) return fail(21, "wayland application was not created");
  if (!app->system_appearance().transparency_effects_enabled) {
    return fail(22, "a bound blur protocol means transparency is on");
  }
  auto window = app->create_window(mica_descriptor(), [](const auto&) {});
  if (!window) return fail(23, "window was not created");
  const auto opaque = (*window)->window_backdrop_state();
  if (!opaque.supported || opaque.applied != WindowBackdrop::none) {
    return fail(24, "creation-time state must not claim blur");
  }
  const auto blur = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
  if (blur.applied != WindowBackdrop::blur ||
      blur.mechanism != "ext_background_effect_v1") {
    return fail(25, "transparent surface must get compositor blur");
  }
  (void)(*window)->apply_window_backdrop(WindowBackdrop::none, true);
  // The requests were flushed; give the server thread time to dispatch.
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (effect.blur_regions_cleared.load() == 0 &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  window->reset();
  app.reset();
  compositor.stop();
  if (effect.effects_created.load() != 1 || effect.blur_regions_set.load() != 1 ||
      effect.blur_regions_cleared.load() != 1) {
    std::cerr << effect.effects_created << ' ' << effect.blur_regions_set << ' '
              << effect.blur_regions_cleared << '\n';
    return fail(26, "compositor must see one effect, one region, one clear");
  }
  return 0;
}
