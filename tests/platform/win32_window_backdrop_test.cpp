#include "cgpui/platform/platform.hpp"
#include "win32_window_backdrop_internal.hpp"

#include <dwmapi.h>

#include <iostream>
#include <variant>

namespace {

using cgpui::WindowBackdrop;

int fail(int code, const char* message) {
  std::cerr << "win32 window backdrop: " << message << '\n';
  return code;
}

int check_plan() {
  using cgpui::Win32BackdropInputs;
  const auto plan = [](Win32BackdropInputs inputs) {
    return cgpui::win32_plan_window_backdrop(inputs);
  };
  const auto old = plan({.requested = WindowBackdrop::mica,
                         .transparent_surface = true, .os_build = 22000});
  if (old.state.supported || old.touch_dwm || old.state.reason.empty()) {
    return fail(1, "builds before 22621 must be unsupported");
  }
  const auto layered = plan({.requested = WindowBackdrop::mica,
                             .transparent_surface = true, .os_build = 22621,
                             .layered = true});
  if (layered.state.supported || layered.touch_dwm) {
    return fail(2, "WS_EX_LAYERED must not be combined with a backdrop");
  }
  const auto ok = plan({.requested = WindowBackdrop::mica_alt,
                        .transparent_surface = true, .os_build = 26100});
  if (ok.state.applied != WindowBackdrop::mica_alt || !ok.extend_frame ||
      ok.backdrop_type != cgpui::win32_backdrop_type_tabbed_window) {
    return fail(3, "mica_alt must map to the tabbed backdrop over the client");
  }
  const auto blur = plan({.requested = WindowBackdrop::blur,
                          .transparent_surface = true, .os_build = 26100});
  if (blur.state.applied != WindowBackdrop::acrylic ||
      blur.backdrop_type != cgpui::win32_backdrop_type_transient_window) {
    return fail(4, "blur must map to acrylic");
  }
  const auto opaque = plan({.requested = WindowBackdrop::mica,
                            .transparent_surface = false, .os_build = 26100});
  if (opaque.state.applied != WindowBackdrop::none || opaque.extend_frame ||
      opaque.state.title_bar != WindowBackdrop::mica || opaque.state.reason.empty()) {
    return fail(5, "an opaque surface gets the title bar material only");
  }
  const auto off = plan({.requested = WindowBackdrop::acrylic,
                         .transparent_surface = true, .os_build = 26100,
                         .transparency_effects_enabled = false});
  if (off.state.applied != WindowBackdrop::none || !off.touch_dwm ||
      off.backdrop_type != cgpui::win32_backdrop_type_none ||
      off.state.reason.empty()) {
    return fail(6, "disabled transparency must clear the backdrop with a reason");
  }
  return 0;
}

DWORD read_type(HWND hwnd) {
  DWORD type = 0xFFFFFFFFU;
  if (FAILED(DwmGetWindowAttribute(
          hwnd, cgpui::win32_system_backdrop_attribute, &type, sizeof(type)))) {
    return 0xFFFFFFFFU;
  }
  return type;
}

} // namespace

int main() {
  if (const int code = check_plan(); code != 0) return code;
  auto app = cgpui::create_platform_application();
  if (!app) return fail(10, "platform application was not created");
  cgpui::WindowDescriptor descriptor;
  descriptor.title = "CGPUI backdrop test";
  descriptor.size = cgpui::Size{.width = 320.0F, .height = 200.0F};
  descriptor.backdrop = WindowBackdrop::mica;
  auto window = (*app)->create_window(descriptor, [](const auto&) {});
  if (!window) return fail(11, "window was not created");
  const cgpui::NativeSurfaceHandle native = (*window)->native_surface();
  const auto* surface = std::get_if<cgpui::Win32SurfaceHandle>(&native);
  if (surface == nullptr || surface->hwnd == nullptr) return fail(12, "no HWND");
  const HWND hwnd = static_cast<HWND>(surface->hwnd);
  const std::uint32_t build = cgpui::win32_os_build_number();
  const bool enabled = (*app)->system_appearance().transparency_effects_enabled;
  std::cout << "build " << build << ", transparency " << enabled << '\n';

  const auto initial = (*window)->window_backdrop_state();
  if (build < cgpui::win32_system_backdrop_min_build) {
    return initial.supported || initial.applied != WindowBackdrop::none
        ? fail(13, "old builds must report unsupported")
        : 0;
  }
  // Created before any renderer: the surface is treated as opaque.
  if (!initial.supported || initial.applied != WindowBackdrop::none ||
      initial.requested != WindowBackdrop::mica || initial.reason.empty()) {
    return fail(14, "creation-time state must not claim the client area");
  }
  if (read_type(hwnd) != (enabled ? cgpui::win32_backdrop_type_main_window
                                  : cgpui::win32_backdrop_type_none)) {
    return fail(15, "creation must set the mica title bar backdrop");
  }
  const auto transparent = (*window)->apply_window_backdrop(WindowBackdrop::mica, true);
  std::cout << "mica: applied " << static_cast<int>(transparent.applied)
            << " mechanism '" << transparent.mechanism << "' reason '"
            << transparent.reason << "'\n";
  if (enabled && (transparent.applied != WindowBackdrop::mica ||
                  !transparent.transparent_surface ||
                  read_type(hwnd) != cgpui::win32_backdrop_type_main_window)) {
    return fail(16, "a transparent surface must get mica behind the client");
  }
  // The appearance monitor calls this for each change; feed it synthetic
  // snapshots.
  cgpui::SystemAppearance appearance = (*app)->system_appearance();
  appearance.transparency_effects_enabled = false;
  cgpui::win32_reevaluate_thread_window_backdrops(appearance);
  const auto disabled = (*window)->window_backdrop_state();
  if (disabled.applied != WindowBackdrop::none || disabled.reason.empty() ||
      read_type(hwnd) != cgpui::win32_backdrop_type_none) {
    return fail(17, "disabled transparency must remove the backdrop");
  }
  appearance.transparency_effects_enabled = true;
  cgpui::win32_reevaluate_thread_window_backdrops(appearance);
  if ((*window)->window_backdrop_state().applied != WindowBackdrop::mica ||
      read_type(hwnd) != cgpui::win32_backdrop_type_main_window) {
    return fail(18, "re-enabled transparency must restore mica");
  }
  const auto acrylic = (*window)->apply_window_backdrop(WindowBackdrop::blur, true);
  if (enabled && (acrylic.applied != WindowBackdrop::acrylic ||
      read_type(hwnd) != cgpui::win32_backdrop_type_transient_window)) {
    return fail(19, "blur must apply acrylic");
  }
  const auto none = (*window)->apply_window_backdrop(WindowBackdrop::none, true);
  if (none.applied != WindowBackdrop::none ||
      read_type(hwnd) != cgpui::win32_backdrop_type_none) {
    return fail(20, "none must clear the backdrop");
  }
  return 0;
}
