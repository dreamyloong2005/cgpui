#pragma once

#include "wayland_protocol_background_effect_internal.hpp"

#include "cgpui/platform/platform_appearance.hpp"
#include "cgpui/platform/platform_window_backdrop.hpp"

#include <cstdint>

namespace cgpui {

// Compositor blur globals, bound only when the registry advertises them.
struct WaylandBackdropGlobals {
  ext_background_effect_manager_v1* ext_manager = nullptr;
  std::uint32_t ext_capabilities = 0;
  org_kde_kwin_blur_manager* kde_manager = nullptr;
};

enum class WaylandBlurProtocol { none, ext_background_effect, kde_blur };

// ext_background_effect_v1 needs its blur capability; KDE blur has none.
[[nodiscard]] WaylandBlurProtocol wayland_blur_protocol(
    const WaylandBackdropGlobals& globals);

// Binds `interface` when it is a blur global; returns whether it was one.
bool wayland_bind_backdrop_global(
    wl_registry* registry,
    std::uint32_t name,
    const char* interface,
    std::uint32_t version,
    WaylandBackdropGlobals& globals);
void wayland_reset_backdrop_globals(WaylandBackdropGlobals& globals);

// The portal has no transparency key: transparency_effects_enabled reports
// whether a compositor blur protocol is bound.
[[nodiscard]] SystemAppearance wayland_backdrop_appearance(
    SystemAppearance appearance,
    const WaylandBackdropGlobals& globals);
[[nodiscard]] SystemAppearanceCallback wayland_backdrop_appearance_callback(
    SystemAppearanceCallback callback,
    const WaylandBackdropGlobals* globals);

struct WaylandBackdropPlan {
  PlatformWindowBackdropState state;
  WaylandBlurProtocol protocol = WaylandBlurProtocol::none;
  bool blur = false;
};

[[nodiscard]] WaylandBackdropPlan wayland_plan_window_backdrop(
    WindowBackdrop requested,
    bool transparent_surface,
    WaylandBlurProtocol protocol);

// Per-surface blur objects. release() must run before the wl_surface is
// destroyed.
class WaylandWindowBackdrop {
 public:
  void attach(
      wl_display* display,
      wl_compositor* compositor,
      wl_surface* surface,
      const WaylandBackdropGlobals* globals);
  PlatformWindowBackdropState apply(
      WindowBackdrop backdrop,
      bool transparent_surface);
  void release();
  [[nodiscard]] const PlatformWindowBackdropState& state() const;

 private:
  void set_blur(WaylandBlurProtocol protocol, bool enabled);

  wl_display* display_ = nullptr;
  wl_compositor* compositor_ = nullptr;
  wl_surface* surface_ = nullptr;
  const WaylandBackdropGlobals* globals_ = nullptr;
  ext_background_effect_surface_v1* ext_effect_ = nullptr;
  org_kde_kwin_blur* kde_blur_ = nullptr;
  PlatformWindowBackdropState state_{
      .backend = "wayland", .reason = "backdrop not applied yet"};
};

} // namespace cgpui
