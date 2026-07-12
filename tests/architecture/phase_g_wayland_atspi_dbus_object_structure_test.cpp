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
  const std::string object_header = read_source(
      "src/platform/linux/wayland_atspi_object_internal.hpp");
  const std::string object = read_source(
      "src/platform/linux/wayland_atspi_object.cpp");
  const std::string dbus_header = read_source(
      "src/platform/linux/wayland_atspi_dbus_internal.hpp");
  const std::string dbus = read_source(
      "src/platform/linux/wayland_atspi_dbus.cpp");
  const std::string messages_header = read_source(
      "src/platform/linux/wayland_atspi_dbus_messages_internal.hpp");
  const std::string messages = read_source(
      "src/platform/linux/wayland_atspi_dbus_messages.cpp");
  const std::string adapter_header = read_source(
      "src/platform/linux/wayland_accessibility_internal.hpp");
  const std::string adapter = read_source(
      "src/platform/linux/wayland_accessibility.cpp");
  const std::string services = read_source(
      "src/platform/linux/wayland_services_internal.hpp");
  const std::string window = read_source(
      "src/platform/linux/wayland_window.cpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_dbus_object_test.cpp");
  const std::string test_support = read_source(
      "tests/platform/wayland_atspi_dbus_test_support.hpp");
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
      &object_header, &object, &dbus_header, &dbus, &messages_header,
      &messages, &adapter_header, &adapter, &services, &window, &behavior,
      &test_support,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(object_header, "struct WaylandAtspiObjectNode") ||
      !contains(object, "build_wayland_atspi_object_nodes(") ||
      !contains(object, "\"/org/a11y/atspi/accessible/\"") ||
      !contains(adapter_header, "WaylandAtspiDbusRegistry dbus_registry_") ||
      !contains(adapter, "dbus_registry_.synchronize(atspi_object_nodes_)")) {
    return 2;
  }
  if (!contains(dbus_header, "struct WaylandAtspiDbusOperations") ||
      !contains(dbus_header, "class WaylandAtspiDbusRegistry") ||
      !contains(dbus, "dbus_connection_register_object_path") ||
      !contains(dbus, "dbus_connection_unregister_object_path") ||
      !contains(dbus, "DBusObjectPathVTable") ||
      !contains(dbus, "registered_paths_") ||
      !contains(dbus, "operations_.send")) {
    return 3;
  }
  if (!contains(messages_header, "struct WaylandAtspiDbusReply") ||
      !contains(messages, "DBUS_INTERFACE_INTROSPECTABLE") ||
      !contains(messages, "org.a11y.atspi.Accessible") ||
      !contains(messages, "org.a11y.atspi.Component") ||
      !contains(messages, "GetInterfaces") ||
      !contains(messages, "dbus_message_new_method_return")) {
    return 4;
  }
  if (!contains(behavior, "DBusMessage*") ||
      !contains(test_support, "message_function(") ||
      !contains(test_support, "WaylandAtspiDbusRecorder") ||
      !contains(behavior, "registration_attempt_count != 2") ||
      !contains(test_support, "unregistered_paths") ||
      !contains(behavior, "org.freedesktop.DBus.Introspectable")) {
    return 5;
  }
  if (!contains(xmake, "add_requires(\"dbus\", {system = true})") ||
      !contains(xmake, "target(\"phase_g_wayland_atspi_dbus_object_test\")") ||
      !contains(xmake,
                "target(\"phase_g_wayland_atspi_dbus_object_structure_test\")") ||
      !contains(xmake, "add_packages(\"wayland\", \"dbus\", \"libxkbcommon\")")) {
    return 6;
  }
  if (contains(services, "#include <dbus/dbus.h>") ||
      contains(window, "DBusConnection") ||
      contains(window, "dbus_connection_register_object_path") ||
      line_count(object_header) > 50 || line_count(object) > 60 ||
      line_count(dbus_header) > 90 || line_count(dbus) > 170 ||
      line_count(messages_header) > 30 || line_count(messages) > 100 ||
      line_count(adapter_header) > 60 || line_count(adapter) > 110 ||
      line_count(services) > 105 || line_count(behavior) > 230) {
    return 7;
  }

  constexpr const char* completion =
      "Phase G Step 627 exposes Linux AT-SPI accessibility objects on an "
      "attached D-Bus connection with deterministic object-path registration, "
      "introspection, interface discovery, removal, teardown, and diagnostics. "
      "Step 628 Linux AT-SPI tree navigation production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 633 Linux AT-SPI disconnect and reconnect production behavior\"")) {
    return 9;
  }
  return 0;
}
