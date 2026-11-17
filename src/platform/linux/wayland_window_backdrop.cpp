#include "wayland_window_internal.hpp"

#include <limits>

namespace cgpui {

void WaylandWindowBackdrop::attach(
    wl_display* display,
    wl_compositor* compositor,
    wl_surface* surface,
    const WaylandBackdropGlobals* globals) {
  display_ = display;
  compositor_ = compositor;
  surface_ = surface;
  globals_ = globals;
}

PlatformWindowBackdropState WaylandWindowBackdrop::apply(
    WindowBackdrop backdrop,
    bool transparent_surface) {
  const WaylandBackdropPlan plan = wayland_plan_window_backdrop(
      backdrop, transparent_surface,
      globals_ == nullptr ? WaylandBlurProtocol::none
                          : wayland_blur_protocol(*globals_));
  state_ = plan.state;
  if (surface_ == nullptr) return state_;
  set_blur(WaylandBlurProtocol::ext_background_effect,
           plan.blur && plan.protocol == WaylandBlurProtocol::ext_background_effect);
  set_blur(WaylandBlurProtocol::kde_blur,
           plan.blur && plan.protocol == WaylandBlurProtocol::kde_blur);
  wl_surface_commit(surface_);
  if (display_ != nullptr) wl_display_flush(display_);
  return state_;
}

void WaylandWindowBackdrop::set_blur(WaylandBlurProtocol protocol, bool enabled) {
  if (protocol == WaylandBlurProtocol::ext_background_effect) {
    if (enabled && ext_effect_ == nullptr && compositor_ != nullptr) {
      ext_effect_ = ext_background_effect_manager_v1_get_background_effect(
          globals_->ext_manager, surface_);
    }
    if (ext_effect_ == nullptr) return;
    wl_region* region = nullptr;
    if (enabled) {
      // The compositor clips the region to the surface, so one region
      // covers the surface at every size.
      region = wl_compositor_create_region(compositor_);
      wl_region_add(region, 0, 0, std::numeric_limits<std::int32_t>::max(),
                    std::numeric_limits<std::int32_t>::max());
    }
    ext_background_effect_surface_v1_set_blur_region(ext_effect_, region);
    if (region != nullptr) wl_region_destroy(region);
    return;
  }
  if (enabled) {
    if (kde_blur_ == nullptr) {
      kde_blur_ = org_kde_kwin_blur_manager_create(globals_->kde_manager, surface_);
    }
    // A null region blurs the whole surface.
    org_kde_kwin_blur_set_region(kde_blur_, nullptr);
    org_kde_kwin_blur_commit(kde_blur_);
  } else if (kde_blur_ != nullptr) {
    if (globals_->kde_manager != nullptr) {
      org_kde_kwin_blur_manager_unset(globals_->kde_manager, surface_);
    }
    org_kde_kwin_blur_release(kde_blur_);
    kde_blur_ = nullptr;
  }
}

void WaylandWindowBackdrop::release() {
  if (ext_effect_ != nullptr) {
    ext_background_effect_surface_v1_destroy(ext_effect_);
    ext_effect_ = nullptr;
  }
  if (kde_blur_ != nullptr) {
    org_kde_kwin_blur_release(kde_blur_);
    kde_blur_ = nullptr;
  }
  surface_ = nullptr;
}

const PlatformWindowBackdropState& WaylandWindowBackdrop::state() const {
  return state_;
}

PlatformWindowBackdropState WaylandWindow::apply_window_backdrop(
    WindowBackdrop backdrop, bool transparent_surface) {
  return backdrop_.apply(backdrop, transparent_surface);
}

PlatformWindowBackdropState WaylandWindow::window_backdrop_state() const {
  return backdrop_.state();
}

void WaylandWindow::attach_backdrop(
    wl_compositor* compositor, const WaylandBackdropGlobals* globals) {
  backdrop_.attach(display_, compositor, surface_, globals);
}

} // namespace cgpui
