#include "wayland_system_application_internal.hpp"
#include "wayland_dpi_awareness_internal.hpp"
#include "wayland_portal_appearance_monitor_internal.hpp"

#include <utility>

namespace cgpui {

WaylandSystemApplication::WaylandSystemApplication()
    : appearance_monitor_(std::make_unique<WaylandPortalAppearanceMonitor>()) {}

WaylandSystemApplication::~WaylandSystemApplication() = default;

// The portal connection is made on first use, so applications that never
// ask for the appearance do not touch the session bus. The portal has no
// transparency key; a bound compositor blur protocol decides it instead.
SystemAppearance WaylandSystemApplication::system_appearance() const {
  return wayland_backdrop_appearance(
      appearance_monitor_->appearance(), backdrop_globals_);
}

void WaylandSystemApplication::set_system_appearance_callback(
    SystemAppearanceCallback callback) {
  appearance_monitor_->set_callback(wayland_backdrop_appearance_callback(
      std::move(callback), &backdrop_globals_));
}

PlatformDpiAwarenessState WaylandSystemApplication::dpi_awareness() const {
  return wayland_dpi_awareness_state(fractional_scale_available_);
}

void WaylandSystemApplication::set_fractional_scale_available(bool available) {
  fractional_scale_available_ = available;
}

WaylandBackdropGlobals& WaylandSystemApplication::backdrop_globals() {
  return backdrop_globals_;
}

WaylandEventSource* WaylandSystemApplication::appearance_event_source() {
  return appearance_monitor_.get();
}

} // namespace cgpui
