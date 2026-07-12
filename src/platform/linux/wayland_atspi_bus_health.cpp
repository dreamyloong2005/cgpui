#include "wayland_atspi_bus_internal.hpp"

namespace cgpui {

bool WaylandAtspiBusConnection::is_connected() {
  if (connection_ == nullptr || !diagnostics_.connected) return false;
  if (operations_.get_is_connected == nullptr ||
      operations_.get_is_connected(connection_)) {
    return true;
  }
  diagnostics_.connected = false;
  diagnostics_.connection_loss_count += 1;
  reconnect_pending_ = true;
  return false;
}

void WaylandAtspiBusConnection::disconnect() {
  if (connection_ == nullptr) return;
  if (operations_.close != nullptr) operations_.close(connection_);
  if (operations_.unref != nullptr) operations_.unref(connection_);
  connection_ = nullptr;
  diagnostics_.connected = false;
  diagnostics_.disconnect_count += 1;
}

DBusConnection* WaylandAtspiBusConnection::connection() const {
  return connection_;
}

WaylandAtspiBusDiagnostics WaylandAtspiBusConnection::diagnostics() const {
  return diagnostics_;
}

} // namespace cgpui
