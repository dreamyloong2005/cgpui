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

std::size_t line_count(const std::string& text) {
  std::size_t lines = 0;
  for (const char value : text) lines += value == '\n' ? 1U : 0U;
  return lines + (!text.empty() && text.back() != '\n' ? 1U : 0U);
}

} // namespace

int main() {
  const std::string event_header = read_source(
      "src/platform/linux/wayland_atspi_events_internal.hpp");
  const std::string event_source = read_source(
      "src/platform/linux/wayland_atspi_events.cpp");
  const std::string adapter_header = read_source(
      "src/platform/linux/wayland_accessibility_internal.hpp");
  const std::string adapter_source = read_source(
      "src/platform/linux/wayland_accessibility.cpp");
  const std::string adapter_bus = read_source(
      "src/platform/linux/wayland_accessibility_bus.cpp");
  const std::string support = read_source(
      "tests/platform/wayland_atspi_dbus_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_text_value_event_test.cpp");
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &event_header, &event_source, &adapter_header, &adapter_source,
      &adapter_bus, &support, &behavior, &manifests};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(event_header, "struct WaylandAtspiEventDiagnostics") ||
      !contains(event_header, "class WaylandAtspiEventPublisher") ||
      !contains(event_header, "missing_object_count") ||
      !contains(event_header, "no_connection_count") ||
      !contains(event_header, "focus_event_count") ||
      !contains(event_header, "send_failure_count")) {
    return 2;
  }
  if (!contains(event_source, "org.a11y.atspi.Event.Object") ||
      !contains(event_source, "PropertyChange") ||
      !contains(event_source, "accessible-value") ||
      !contains(event_source, "TextChanged") ||
      !contains(event_source, "insert") ||
      !contains(event_source, "utf8_code_point_count") ||
      !contains(event_source, "DBUS_TYPE_VARIANT") ||
      !contains(event_source, "DBUS_TYPE_STRUCT")) {
    return 3;
  }
  if (!contains(event_source, "diagnostics_.missing_object_count += 1") ||
      !contains(event_source, "diagnostics_.no_connection_count += 1") ||
      !contains(event_source, "diagnostics_.focus_event_count += 1") ||
      !contains(event_source, "diagnostics_.send_failure_count += 1")) {
    return 4;
  }
  if (!contains(adapter_header, "WaylandAtspiEventPublisher event_publisher_") ||
      !contains(adapter_header, "event_diagnostics() const") ||
      !contains(adapter_source, "dbus_registry_.synchronize(atspi_object_nodes_);") ||
      !contains(adapter_source,
                "event_publisher_.publish(atspi_object_nodes_, live_updates_);") ||
      !contains(adapter_bus, "event_publisher_.attach(connection, operations)") ||
      !contains(adapter_bus, "event_publisher_.detach()")) {
    return 5;
  }
  if (!contains(support, "std::vector<DBusMessage*> sent_messages") ||
      !contains(support, "bool send_result = true") ||
      !contains(support, "return active_->send_result")) {
    return 6;
  }
  if (!contains(behavior, "siiv(so)") ||
      !contains(behavior, "text.detail2 != 2") ||
      !contains(behavior, "diagnostics.no_connection_count != 1") ||
      !contains(behavior, "diagnostics.send_failure_count != 1")) {
    return 7;
  }
  if (!contains(manifests,
                "\"wayland_atspi_text_value_event_test.cpp\"") ||
      !contains(
          manifests,
          "\"phase_g_wayland_atspi_text_value_event_structure_test.cpp\"")) {
    return 8;
  }
  if (line_count(event_header) > 50 || line_count(event_source) > 150 ||
      line_count(adapter_header) > 60 || line_count(adapter_source) > 110 ||
      line_count(adapter_bus) > 100 || line_count(support) > 250 ||
      line_count(behavior) > 210) {
    return 9;
  }

  return 0;
}
