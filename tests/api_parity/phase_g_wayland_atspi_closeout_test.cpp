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

} // namespace

int main() {
  const std::string objects = read_source(
      "tests/architecture/phase_g_wayland_atspi_dbus_object_structure_test.cpp");
  const std::string navigation = read_source(
      "tests/architecture/phase_g_wayland_atspi_tree_navigation_structure_test.cpp");
  const std::string roles = read_source(
      "tests/architecture/phase_g_wayland_atspi_role_state_structure_test.cpp");
  const std::string text_value = read_source(
      "tests/architecture/phase_g_wayland_atspi_text_value_event_structure_test.cpp");
  const std::string focus = read_source(
      "tests/architecture/phase_g_wayland_atspi_focus_event_structure_test.cpp");
  const std::string bus = read_source(
      "tests/architecture/phase_g_wayland_atspi_bus_connection_structure_test.cpp");
  const std::string reconnect = read_source(
      "tests/architecture/phase_g_wayland_atspi_bus_reconnect_structure_test.cpp");
  const std::string platform_source = read_source(
      "tests/architecture/platform_source_structure_test.cpp");
  const std::string wayland_source = read_source(
      "tests/architecture/wayland_window_source_test.cpp");
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
      &objects, &navigation, &roles, &text_value, &focus, &bus, &reconnect,
      &platform_source, &wayland_source, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(objects, "register_object_path") ||
      !contains(navigation, "GetChildAtIndex") ||
      !contains(roles, "GetState") ||
      !contains(text_value, "TextChanged") ||
      !contains(focus, "StateChanged") ||
      !contains(bus, "org.a11y.Bus") ||
      !contains(reconnect, "reconnect_success_count")) {
    return 2;
  }
  const char* targets[]{
      "phase_g_wayland_atspi_dbus_object_test",
      "phase_g_wayland_atspi_tree_navigation_test",
      "phase_g_wayland_atspi_role_state_test",
      "phase_g_wayland_atspi_text_value_event_test",
      "phase_g_wayland_atspi_focus_event_test",
      "phase_g_wayland_atspi_bus_connection_test",
      "phase_g_wayland_atspi_bus_reconnect_test",
      "phase_g_wayland_atspi_dbus_object_structure_test",
      "phase_g_wayland_atspi_tree_navigation_structure_test",
      "phase_g_wayland_atspi_role_state_structure_test",
      "phase_g_wayland_atspi_text_value_event_structure_test",
      "phase_g_wayland_atspi_focus_event_structure_test",
      "phase_g_wayland_atspi_bus_connection_structure_test",
      "phase_g_wayland_atspi_bus_reconnect_structure_test"};
  for (const char* target : targets) {
    if (!contains(xmake, (std::string{"target(\""} + target + "\")").c_str())) {
      return 3;
    }
  }
  const char* sources[]{
      "wayland_atspi_object.cpp", "wayland_atspi_dbus.cpp",
      "wayland_atspi_dbus_messages.cpp", "wayland_atspi_dbus_navigation.cpp",
      "wayland_atspi_role_state.cpp", "wayland_atspi_events.cpp",
      "wayland_atspi_bus.cpp", "wayland_atspi_bus_health.cpp",
      "wayland_accessibility.cpp", "wayland_accessibility_bus.cpp"};
  for (const char* source : sources) {
    if (!contains(wayland_source, source)) return 4;
  }
  if (!contains(platform_source, "WaylandAtspiAccessibilityAdapter") ||
      !contains(platform_source, "WaylandAtspiBusConnection::is_connected") ||
      !contains(xmake, "target(\"phase_g_wayland_atspi_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_g_wayland_atspi_closeout_test.cpp") ||
      !contains(roadmap, "- [x] Steps 627-634: Implement Linux AT-SPI")) {
    return 5;
  }

  constexpr const char* completion =
      "Phase G Step 634 audits and closes the Linux AT-SPI production band, "
      "freezing D-Bus object exposure, navigation, roles/states, text/value/"
      "focus events, accessibility-bus discovery, reconnect behavior, "
      "diagnostics, and modular source evidence. Step 635 async task pool "
      "production depth is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 6;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 643 animation transition production behavior\"")) {
    return 7;
  }
  return 0;
}
