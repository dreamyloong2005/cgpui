#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string bus_header = read_source(
      "src/platform/linux/wayland_atspi_bus_internal.hpp");
  const std::string bus_source = read_source(
      "src/platform/linux/wayland_atspi_bus.cpp");
  const std::string bus_health = read_source(
      "src/platform/linux/wayland_atspi_bus_health.cpp");
  const std::string adapter_header = read_source(
      "src/platform/linux/wayland_accessibility_internal.hpp");
  const std::string adapter_source = read_source(
      "src/platform/linux/wayland_accessibility.cpp");
  const std::string adapter_bus = read_source(
      "src/platform/linux/wayland_accessibility_bus.cpp");
  const std::string services = read_source(
      "src/platform/linux/wayland_services_internal.hpp");
  const std::string window = read_source(
      "src/platform/linux/wayland_window.cpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_bus_connection_test.cpp");
  const std::string bus_test_support = read_source(
      "tests/platform/wayland_atspi_bus_test_support.hpp");
  const std::string platform_inventory = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string wayland_inventory = read_source(
      "tests/architecture/wayland_window_source_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_wayland_atspi_focus_event_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source(
      "docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source(
      "docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &bus_header, &bus_source, &bus_health, &adapter_header, &adapter_source,
      &adapter_bus, &services, &window, &behavior, &bus_test_support, &platform_inventory,
      &wayland_inventory, &previous, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(bus_header, "struct WaylandAtspiBusOperations") ||
      !contains(bus_header, "struct WaylandAtspiBusDiagnostics") ||
      !contains(bus_header, "class WaylandAtspiBusConnection") ||
      !contains(bus_header, "WaylandAtspiBusConnection(const WaylandAtspiBusConnection&) = delete")) {
    return 2;
  }
  if (!contains(bus_source, "dbus_bus_get(DBUS_BUS_SESSION") ||
      !contains(bus_source, "\"org.a11y.Bus\"") ||
      !contains(bus_source, "\"/org/a11y/bus\"") ||
      !contains(bus_source, "\"GetAddress\"") ||
      !contains(bus_source, "dbus_connection_open_private") ||
      !contains(bus_source, "dbus_bus_register") ||
      !contains(bus_source, "dbus_connection_set_exit_on_disconnect") ||
      !contains(bus_source, "dbus_connection_close") ||
      !contains(bus_source, "dbus_connection_unref")) {
    return 3;
  }
  if (!contains(bus_source, "diagnostics_.discovery_failure_count += 1") ||
      !contains(bus_source, "diagnostics_.connection_failure_count += 1") ||
      !contains(bus_health, "diagnostics_.disconnect_count += 1")) {
    return 4;
  }
  if (!contains(adapter_header, "WaylandAtspiBusConnection bus_connection_") ||
      !contains(adapter_header, "automatic_connection_enabled_") ||
      !contains(adapter_bus, "ensure_dbus_connection(") ||
      !contains(adapter_bus, "bus_connection_.connect(bus_operations)") ||
      !contains(adapter_bus, "dbus_registry_.attach(bus_connection_.connection()") ||
      !contains(adapter_bus, "event_publisher_.attach(bus_connection_.connection()")) {
    return 5;
  }
  if (!contains(services, "wayland_atspi_ensure_dbus_connection(") ||
      !contains(window, "wayland_atspi_ensure_dbus_connection(*atspi_accessibility_)") ||
      window.find("wayland_atspi_ensure_dbus_connection(*atspi_accessibility_)") >
          window.find("wayland_atspi_update_accessibility_tree(")) {
    return 6;
  }
  if (!contains(bus_test_support, "org.a11y.Bus") ||
      !contains(bus_test_support, "unix:path=/tmp/cgpui-atspi") ||
      !contains(behavior, "diagnostics.discovery_attempt_count != 1") ||
      !contains(behavior, "discovery_failure_count != 1") ||
      !contains(behavior, "connection_failure_count != 1") ||
      !contains(behavior, "failed_register.close_calls != 1")) {
    return 7;
  }
  const char* bus_source_path = "src/platform/linux/wayland_atspi_bus.cpp";
  const char* bus_health_path =
      "src/platform/linux/wayland_atspi_bus_health.cpp";
  const char* adapter_bus_path =
      "src/platform/linux/wayland_accessibility_bus.cpp";
  if (!contains(platform_inventory, bus_source_path) ||
      !contains(platform_inventory, bus_health_path) ||
      !contains(platform_inventory, adapter_bus_path) ||
      !contains(wayland_inventory, bus_source_path) ||
      !contains(wayland_inventory, bus_health_path) ||
      !contains(wayland_inventory, adapter_bus_path)) {
    return 8;
  }
  if (!contains(xmake, "target(\"phase_g_wayland_atspi_bus_connection_test\")") ||
      !contains(
          xmake,
          "target(\"phase_g_wayland_atspi_bus_connection_structure_test\")")) {
    return 9;
  }
  if (line_count(bus_header) > 70 || line_count(bus_source) > 180 ||
      line_count(bus_health) > 70 ||
      line_count(adapter_header) > 60 || line_count(adapter_source) > 80 ||
      line_count(adapter_bus) > 100 || line_count(services) > 110 ||
      line_count(window) > 180 || line_count(behavior) > 100 ||
      line_count(bus_test_support) > 130) {
    return 10;
  }

  constexpr const char* completion =
      "Phase G Step 632 discovers the Linux AT-SPI accessibility bus through "
      "org.a11y.Bus, opens and registers an owned private connection, attaches "
      "it lazily on first accessibility update, and reports discovery/connection "
      "lifecycle diagnostics. Step 633 Linux AT-SPI disconnect and reconnect "
      "production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 11;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 664 GPUI-style platform service fake production behavior\"")) {
    return 12;
  }
  return 0;
}
