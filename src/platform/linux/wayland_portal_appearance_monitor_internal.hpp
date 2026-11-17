#pragma once

#include "wayland_event_source_internal.hpp"
#include "wayland_portal_settings_internal.hpp"

#include <cstddef>
#include <string>

namespace cgpui {

struct WaylandPortalAppearanceDiagnostics {
  std::size_t connection_attempt_count = 0;
  std::size_t setting_read_count = 0;
  std::size_t deprecated_read_count = 0;
  std::size_t signal_count = 0;
  std::size_t ignored_signal_count = 0;
  bool connected = false;
  std::string unavailable_reason;
};

// Reads the appearance settings from xdg-desktop-portal on the session bus
// (or on `bus_address`, used by tests) on first use, then keeps the snapshot
// current from SettingChanged signals. The Wayland event loop polls the
// connection through WaylandEventSource, so callbacks run on the UI thread.
// Availability is decided once: if the portal is missing, the snapshot stays
// unsupported with the reason recorded.
class WaylandPortalAppearanceMonitor final : public WaylandEventSource {
 public:
  explicit WaylandPortalAppearanceMonitor(std::string bus_address = {});
  ~WaylandPortalAppearanceMonitor() override;
  WaylandPortalAppearanceMonitor(const WaylandPortalAppearanceMonitor&) =
      delete;
  WaylandPortalAppearanceMonitor& operator=(
      const WaylandPortalAppearanceMonitor&) = delete;

  [[nodiscard]] SystemAppearance appearance();
  void set_callback(SystemAppearanceCallback callback);
  [[nodiscard]] const WaylandPortalAppearanceDiagnostics& diagnostics() const;

  [[nodiscard]] int descriptor() const override;
  [[nodiscard]] bool has_pending() const override;
  void dispatch() override;

 private:
  void ensure_connected();
  bool connect_bus(DBusError& error);
  bool subscribe(DBusError& error);
  bool read_snapshot();
  WaylandPortalReadStatus read_setting(
      SystemAppearance& snapshot,
      const char* name_space,
      const char* key);
  void handle_setting_changed(DBusMessage* message);
  void mark_unavailable(std::string reason);
  void release_connection();
  static DBusHandlerResult filter(
      DBusConnection* connection,
      DBusMessage* message,
      void* data);

  std::string bus_address_;
  DBusConnection* connection_ = nullptr;
  bool attempted_ = false;
  bool filter_installed_ = false;
  bool use_deprecated_read_ = false;
  std::string last_error_;
  SystemAppearance appearance_;
  SystemAppearanceCallback callback_;
  WaylandPortalAppearanceDiagnostics diagnostics_;
};

} // namespace cgpui
