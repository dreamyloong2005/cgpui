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
  const std::string event_header = read_source(
      "src/platform/linux/wayland_atspi_events_internal.hpp");
  const std::string event_source = read_source(
      "src/platform/linux/wayland_atspi_events.cpp");
  const std::string adapter_header = read_source(
      "src/platform/linux/wayland_accessibility_internal.hpp");
  const std::string adapter_source = read_source(
      "src/platform/linux/wayland_accessibility.cpp");
  const std::string support = read_source(
      "tests/platform/wayland_atspi_dbus_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_text_value_event_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_wayland_atspi_role_state_structure_test.cpp");
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
      &event_header, &event_source, &adapter_header, &adapter_source, &support,
      &behavior, &previous, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
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
      !contains(adapter_source, "event_publisher_.attach(connection, operations)") ||
      !contains(adapter_source, "event_publisher_.detach()")) {
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
  if (!contains(xmake,
                "target(\"phase_g_wayland_atspi_text_value_event_test\")") ||
      !contains(
          xmake,
          "target(\"phase_g_wayland_atspi_text_value_event_structure_test\")")) {
    return 8;
  }
  if (line_count(event_header) > 50 || line_count(event_source) > 150 ||
      line_count(adapter_header) > 60 || line_count(adapter_source) > 110 ||
      line_count(support) > 250 || line_count(behavior) > 210) {
    return 9;
  }

  constexpr const char* completion =
      "Phase G Step 630 publishes Linux AT-SPI value property and text change "
      "events from runtime accessibility live updates with standard Object "
      "event signals, UTF-8 text lengths, source references, and publication "
      "diagnostics. Step 631 Linux AT-SPI focus event production behavior is "
      "next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 632 Linux AT-SPI accessibility bus discovery and connection production behavior\"")) {
    return 11;
  }
  return 0;
}
