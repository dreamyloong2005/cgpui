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
  const std::string adapter_bus = read_source(
      "src/platform/linux/wayland_accessibility_bus.cpp");
  const std::string bus_test_support = read_source(
      "tests/platform/wayland_atspi_bus_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_bus_reconnect_test.cpp");
  const std::string platform_inventory = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string wayland_inventory = read_source(
      "tests/architecture/wayland_window_source_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_wayland_atspi_bus_connection_structure_test.cpp");
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
      &bus_header, &bus_source, &bus_health, &adapter_header, &adapter_bus,
      &bus_test_support, &behavior, &platform_inventory, &wayland_inventory,
      &previous, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(bus_header, "get_is_connected") ||
      !contains(bus_header, "connection_loss_count") ||
      !contains(bus_header, "reconnect_attempt_count") ||
      !contains(bus_header, "reconnect_success_count") ||
      !contains(bus_header, "reconnect_failure_count") ||
      !contains(bus_header, "bool is_connected()") ||
      !contains(bus_header, "bool reconnect_pending_")) {
    return 2;
  }
  if (!contains(bus_health, "operations_.get_is_connected(connection_)") ||
      !contains(bus_health, "diagnostics_.connection_loss_count += 1") ||
      !contains(bus_health, "reconnect_pending_ = true") ||
      !contains(bus_source, "diagnostics_.reconnect_attempt_count += 1") ||
      !contains(bus_source, "diagnostics_.reconnect_success_count += 1") ||
      !contains(bus_source, "diagnostics_.reconnect_failure_count += 1")) {
    return 3;
  }
  const auto detach = adapter_bus.find("event_publisher_.detach()");
  const auto disconnect = adapter_bus.find("bus_connection_.disconnect()");
  const auto reconnect = adapter_bus.find("bus_connection_.connect(bus_operations)");
  if (!contains(adapter_header, "automatic_connection_enabled_ = true") ||
      !contains(adapter_bus, "bus_connection_.is_connected()") ||
      !contains(adapter_bus, "automatic_connection_enabled_ = false") ||
      detach == std::string::npos || disconnect == std::string::npos ||
      reconnect == std::string::npos || !(detach < disconnect && disconnect < reconnect)) {
    return 4;
  }
  if (!contains(bus_test_support, "report_disconnect_once") ||
      !contains(bus_test_support, "get_is_connected") ||
      !contains(behavior, "failed.reconnect_failure_count != 1") ||
      !contains(behavior, "recovered.reconnect_success_count != 1") ||
      !contains(behavior, "dbus_recorder.unregistered_paths") ||
      !contains(behavior, "injected->attach_dbus") ||
      !contains(behavior, "bus_recorder.discovery_calls != discovery_calls")) {
    return 5;
  }
  constexpr const char* health_path =
      "src/platform/linux/wayland_atspi_bus_health.cpp";
  if (!contains(platform_inventory, health_path) ||
      !contains(wayland_inventory, health_path) ||
      !contains(xmake, "target(\"phase_g_wayland_atspi_bus_reconnect_test\")") ||
      !contains(xmake, "target(\"phase_g_wayland_atspi_bus_reconnect_structure_test\")")) {
    return 6;
  }
  if (!contains(previous, "Phase G Step 632 discovers the Linux AT-SPI") ||
      !contains(previous, "wayland_atspi_bus_health.cpp")) {
    return 7;
  }
  if (line_count(bus_header) > 70 || line_count(bus_source) > 160 ||
      line_count(bus_health) > 70 || line_count(adapter_header) > 65 ||
      line_count(adapter_bus) > 110 || line_count(bus_test_support) > 130 ||
      line_count(behavior) > 100) {
    return 8;
  }

  constexpr const char* completion =
      "Phase G Step 633 detects owned Linux AT-SPI bus connection loss, "
      "detaches stale object and event registrations, retries discovery and "
      "registration until recovery, resynchronizes the accessibility tree, "
      "and reports reconnect lifecycle diagnostics. Step 634 Linux AT-SPI "
      "production closeout audit is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 648 animation frame pacing production behavior\"")) {
    return 10;
  }
  return 0;
}
