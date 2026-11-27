#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
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
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/platform/mcpp.toml");
  const std::string* required[]{
      &objects,
      &navigation,
      &roles,
      &text_value,
      &focus,
      &bus,
      &reconnect,
      &platform_source,
      &wayland_source,
      &manifests};
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
      "wayland_atspi_dbus_object_test.cpp",
      "wayland_atspi_tree_navigation_test.cpp",
      "wayland_atspi_role_state_test.cpp",
      "wayland_atspi_text_value_event_test.cpp",
      "wayland_atspi_focus_event_test.cpp",
      "wayland_atspi_bus_connection_test.cpp",
      "wayland_atspi_bus_reconnect_test.cpp",
      "phase_g_wayland_atspi_dbus_object_structure_test.cpp",
      "phase_g_wayland_atspi_tree_navigation_structure_test.cpp",
      "phase_g_wayland_atspi_role_state_structure_test.cpp",
      "phase_g_wayland_atspi_text_value_event_structure_test.cpp",
      "phase_g_wayland_atspi_focus_event_structure_test.cpp",
      "phase_g_wayland_atspi_bus_connection_structure_test.cpp",
      "phase_g_wayland_atspi_bus_reconnect_structure_test.cpp"};
  for (const char* target : targets) {
    if (!contains(manifests, (std::string{"\""} + target + "\"").c_str())) {
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
      !contains(manifests, "\"phase_g_wayland_atspi_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_wayland_atspi_closeout_test.cpp\"")) {
    return 5;
  }

  return 0;
}
