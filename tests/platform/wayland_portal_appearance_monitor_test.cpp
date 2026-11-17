#include "wayland_fake_portal.hpp"
#include "wayland_portal_appearance_monitor_internal.hpp"

#include <chrono>
#include <iostream>
#include <poll.h>
#include <vector>

namespace {

using cgpui::SystemAppearanceChange;
using cgpui::SystemAppearanceChangeSource;
using cgpui::SystemAppearanceMode;
using cgpui::WaylandPortalAppearanceMonitor;
using namespace cgpui::test;

constexpr const char* appearance = "org.freedesktop.appearance";
constexpr const char* gnome = "org.gnome.desktop.interface";

int fail(int code, const char* message) {
  std::cerr << "wayland portal appearance monitor: " << message << '\n';
  return code;
}

// Drives the monitor the way the Wayland event loop does, until `count`
// changes arrived or three seconds passed.
bool pump_until(
    WaylandPortalAppearanceMonitor& monitor,
    const std::vector<SystemAppearanceChange>& seen,
    std::size_t count) {
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (seen.size() < count && std::chrono::steady_clock::now() < deadline) {
    pollfd fd{.fd = monitor.descriptor(), .events = POLLIN, .revents = 0};
    const int timeout = monitor.has_pending() ? 0 : 50;
    if (poll(&fd, 1, timeout) >= 0) monitor.dispatch();
  }
  return seen.size() >= count;
}

int check_missing_portal(const PrivateSessionBus& bus) {
  WaylandPortalAppearanceMonitor monitor(bus.address());
  const cgpui::SystemAppearance snapshot = monitor.appearance();
  if (snapshot.supported || snapshot.backend != "wayland" ||
      snapshot.mode != SystemAppearanceMode::light ||
      snapshot.accent_color_available || snapshot.unsupported_reason.empty() ||
      monitor.descriptor() != -1 ||
      monitor.diagnostics().unavailable_reason.find(
          "org.freedesktop.DBus.Error.ServiceUnknown") == std::string::npos) {
    std::cerr << snapshot.unsupported_reason << '\n';
    return fail(1, "missing portal must be a typed unsupported snapshot");
  }
  (void)monitor.appearance();
  if (monitor.diagnostics().connection_attempt_count != 1) {
    return fail(2, "availability must be decided once");
  }
  return 0;
}

int check_portal(const PrivateSessionBus& bus, bool supports_read_one) {
  FakePortal portal(bus.address(), supports_read_one);
  portal.set_value(appearance, "color-scheme", portal_uint32(1));
  portal.set_value(appearance, "accent-color", portal_rgb(0.0, 0.5, 1.0));
  portal.set_value(gnome, "enable-animations", portal_boolean(false));
  if (!portal.start()) return fail(10, "fake portal did not start");

  WaylandPortalAppearanceMonitor monitor(bus.address());
  std::vector<SystemAppearanceChange> seen;
  monitor.set_callback(
      [&](const SystemAppearanceChange& change) { seen.push_back(change); });
  const cgpui::SystemAppearance snapshot = monitor.appearance();
  if (!snapshot.supported || snapshot.backend != "wayland" ||
      snapshot.mode != SystemAppearanceMode::dark ||
      !snapshot.accent_color_available || snapshot.accent_color.b != 1.0F ||
      snapshot.high_contrast || snapshot.client_area_animation_enabled ||
      snapshot.transparency_effects_enabled ||
      !snapshot.unsupported_reason.empty() || monitor.descriptor() == -1) {
    std::cerr << snapshot.unsupported_reason << '\n';
    return fail(11, "portal snapshot not read (contrast absent = false)");
  }
  const bool fallback_used = portal.read_calls() == 4 &&
                             portal.read_one_calls() == 1 &&
                             monitor.diagnostics().deprecated_read_count == 4;
  const bool read_one_used =
      portal.read_one_calls() == 4 && portal.read_calls() == 0;
  if (supports_read_one ? !read_one_used : !fallback_used) {
    return fail(12, "ReadOne / deprecated Read selection is wrong");
  }

  portal.emit(appearance, "color-scheme", portal_uint32(2));
  portal.emit(appearance, "color-scheme", portal_uint32(2));
  portal.emit(appearance, "gtk-theme", portal_uint32(0));
  portal.emit(appearance, "contrast", portal_uint32(1));
  portal.emit(appearance, "accent-color", portal_rgb(2.0, 0.0, 0.0));
  if (!pump_until(monitor, seen, 4)) return fail(13, "changes not delivered");
  if (seen[0].source != SystemAppearanceChangeSource::color_set ||
      !seen[0].changed ||
      seen[0].appearance.mode != SystemAppearanceMode::light ||
      seen[1].source != SystemAppearanceChangeSource::color_set ||
      seen[1].changed ||
      seen[2].source != SystemAppearanceChangeSource::high_contrast ||
      !seen[2].changed || !seen[2].appearance.high_contrast ||
      seen[3].source != SystemAppearanceChangeSource::colorization ||
      !seen[3].changed || seen[3].appearance.accent_color_available) {
    return fail(14, "change sources or changed flags are wrong");
  }
  if (monitor.diagnostics().signal_count != 5 ||
      monitor.diagnostics().ignored_signal_count != 1 ||
      !(monitor.appearance() == seen[3].appearance)) {
    return fail(15, "unknown keys must be ignored without a callback");
  }
  return 0;
}

} // namespace

int main() {
  PrivateSessionBus bus;
  if (!bus.start()) {
    std::cout << "skipped: dbus-daemon is not available\n";
    return 0;
  }
  if (const int code = check_missing_portal(bus); code != 0) return code;
  if (const int code = check_portal(bus, true); code != 0) return code;
  return check_portal(bus, false);
}
