#include "wayland_backdrop_internal.hpp"

#include <string_view>
#include <utility>

namespace cgpui {
namespace {

void handle_capabilities(
    void* data, ext_background_effect_manager_v1*, std::uint32_t flags) {
  static_cast<WaylandBackdropGlobals*>(data)->ext_capabilities = flags;
}

} // namespace

WaylandBlurProtocol wayland_blur_protocol(
    const WaylandBackdropGlobals& globals) {
  if (globals.ext_manager != nullptr &&
      (globals.ext_capabilities &
       ext_background_effect_manager_v1_capability_blur) != 0) {
    return WaylandBlurProtocol::ext_background_effect;
  }
  return globals.kde_manager != nullptr ? WaylandBlurProtocol::kde_blur
                                        : WaylandBlurProtocol::none;
}

bool wayland_bind_backdrop_global(
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version,
    WaylandBackdropGlobals& globals) {
  (void)version;
  const std::string_view interface_name(interface);
  if (interface_name == ext_background_effect_manager_v1_interface.name &&
      globals.ext_manager == nullptr) {
    globals.ext_manager = static_cast<ext_background_effect_manager_v1*>(
        wl_registry_bind(
            registry, name, &ext_background_effect_manager_v1_interface, 1));
    static const ext_background_effect_manager_v1_listener listener{
        .capabilities = &handle_capabilities,
    };
    ext_background_effect_manager_v1_add_listener(
        globals.ext_manager, &listener, &globals);
    return true;
  }
  if (interface_name == org_kde_kwin_blur_manager_interface.name &&
      globals.kde_manager == nullptr) {
    globals.kde_manager = static_cast<org_kde_kwin_blur_manager*>(
        wl_registry_bind(registry, name, &org_kde_kwin_blur_manager_interface, 1));
    return true;
  }
  return false;
}

void wayland_reset_backdrop_globals(WaylandBackdropGlobals& globals) {
  if (globals.ext_manager != nullptr) {
    ext_background_effect_manager_v1_destroy(globals.ext_manager);
  }
  if (globals.kde_manager != nullptr) {
    org_kde_kwin_blur_manager_destroy(globals.kde_manager);
  }
  globals = WaylandBackdropGlobals{};
}

SystemAppearance wayland_backdrop_appearance(
    SystemAppearance appearance,
    const WaylandBackdropGlobals& globals) {
  appearance.transparency_effects_enabled =
      wayland_blur_protocol(globals) != WaylandBlurProtocol::none;
  return appearance;
}

SystemAppearanceCallback wayland_backdrop_appearance_callback(
    SystemAppearanceCallback callback,
    const WaylandBackdropGlobals* globals) {
  if (!callback) return {};
  return [callback = std::move(callback), globals](
             const SystemAppearanceChange& change) {
    SystemAppearanceChange patched = change;
    patched.appearance =
        wayland_backdrop_appearance(std::move(patched.appearance), *globals);
    callback(patched);
  };
}

WaylandBackdropPlan wayland_plan_window_backdrop(
    WindowBackdrop requested,
    bool transparent_surface,
    WaylandBlurProtocol protocol) {
  WaylandBackdropPlan plan{
      .state = PlatformWindowBackdropState{
          .supported = protocol != WaylandBlurProtocol::none,
          .transparent_surface = transparent_surface,
          .backend = "wayland",
          .requested = requested},
      .protocol = protocol};
  PlatformWindowBackdropState& state = plan.state;
  if (protocol == WaylandBlurProtocol::ext_background_effect) {
    state.mechanism = "ext_background_effect_v1";
  } else if (protocol == WaylandBlurProtocol::kde_blur) {
    state.mechanism = "org_kde_kwin_blur";
  }
  if (requested == WindowBackdrop::none) return plan;
  if (protocol == WaylandBlurProtocol::none) {
    state.reason = "compositor advertises neither ext_background_effect_"
                   "manager_v1 with the blur capability nor "
                   "org_kde_kwin_blur_manager";
    return plan;
  }
  if (!transparent_surface) {
    state.reason = "renderer surface does not composite with alpha; blur "
                   "behind it would not be visible";
    return plan;
  }
  // Mica, mica_alt, and acrylic have no Wayland equivalent; blur is the
  // closest compositor material.
  plan.blur = true;
  state.applied = WindowBackdrop::blur;
  return plan;
}

} // namespace cgpui
