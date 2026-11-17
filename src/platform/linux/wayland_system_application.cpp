#include "wayland_system_application_internal.hpp"
#include "wayland_dpi_awareness_internal.hpp"
#include "wayland_portal_appearance_monitor_internal.hpp"

#include <utility>

namespace cgpui {

WaylandSystemApplication::WaylandSystemApplication()
    : appearance_monitor_(std::make_unique<WaylandPortalAppearanceMonitor>()) {}

WaylandSystemApplication::~WaylandSystemApplication() = default;

// The portal connection is made on first use, so applications that never
// ask for the appearance do not touch the session bus.
SystemAppearance WaylandSystemApplication::system_appearance() const {
  return appearance_monitor_->appearance();
}

void WaylandSystemApplication::set_system_appearance_callback(
    SystemAppearanceCallback callback) {
  appearance_monitor_->set_callback(std::move(callback));
}

PlatformDpiAwarenessState WaylandSystemApplication::dpi_awareness() const {
  return wayland_dpi_awareness_state(fractional_scale_available_);
}

void WaylandSystemApplication::set_fractional_scale_available(bool available) {
  fractional_scale_available_ = available;
}

WaylandEventSource* WaylandSystemApplication::appearance_event_source() {
  return appearance_monitor_.get();
}

} // namespace cgpui
