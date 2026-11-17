#pragma once

#include "wayland_internal.hpp"

#include <memory>

namespace cgpui {

class WaylandPortalAppearanceMonitor;

// Process-wide Wayland desktop state shared by the application: the system
// appearance read from xdg-desktop-portal over the session bus, and the DPI
// awareness implied by the scale protocols the compositor advertises.
class WaylandSystemApplication : public PlatformApplication {
 public:
  WaylandSystemApplication();
  ~WaylandSystemApplication() override;
  WaylandSystemApplication(const WaylandSystemApplication&) = delete;
  WaylandSystemApplication& operator=(const WaylandSystemApplication&) = delete;

  [[nodiscard]] SystemAppearance system_appearance() const override;
  void set_system_appearance_callback(
      SystemAppearanceCallback callback) override;
  [[nodiscard]] PlatformDpiAwarenessState dpi_awareness() const override;

 protected:
  void set_fractional_scale_available(bool available);
  [[nodiscard]] WaylandEventSource* appearance_event_source();
  // Compositor blur globals; they also decide
  // SystemAppearance::transparency_effects_enabled.
  [[nodiscard]] WaylandBackdropGlobals& backdrop_globals();

 private:
  std::unique_ptr<WaylandPortalAppearanceMonitor> appearance_monitor_;
  bool fractional_scale_available_ = false;
  WaylandBackdropGlobals backdrop_globals_;
};

} // namespace cgpui
