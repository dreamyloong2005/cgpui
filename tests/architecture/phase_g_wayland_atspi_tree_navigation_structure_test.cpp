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
  const std::string messages = read_source(
      "src/platform/linux/wayland_atspi_dbus_messages.cpp");
  const std::string navigation_header = read_source(
      "src/platform/linux/wayland_atspi_dbus_navigation_internal.hpp");
  const std::string navigation = read_source(
      "src/platform/linux/wayland_atspi_dbus_navigation.cpp");
  const std::string adapter = read_source(
      "src/platform/linux/wayland_accessibility.cpp");
  const std::string test_support = read_source(
      "tests/platform/wayland_atspi_dbus_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_tree_navigation_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_wayland_atspi_dbus_object_structure_test.cpp");
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
      &object_header, &object, &dbus_header, &dbus, &messages,
      &navigation_header, &navigation, &adapter, &test_support, &behavior,
      &previous, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(object_header, "child_object_paths") ||
      !contains(object_header, "index_in_parent") ||
      !contains(object, "object_indices") ||
      !contains(object, "parent_object.child_object_paths.push_back") ||
      !contains(adapter, "set_root_object_path(")) {
    return 2;
  }
  if (!contains(dbus_header, "get_unique_name") ||
      !contains(dbus_header, "root_object_path_") ||
      !contains(dbus, "dbus_bus_get_unique_name") ||
      !contains(dbus, "objects_.at(path)") ||
      !contains(dbus, "root_object_path_")) {
    return 3;
  }
  if (!contains(navigation_header, "wayland_atspi_navigation_reply_for(") ||
      !contains(navigation, "DBUS_INTERFACE_PROPERTIES") ||
      !contains(navigation, "\"Parent\"") ||
      !contains(navigation, "\"ChildCount\"") ||
      !contains(navigation, "\"GetChildAtIndex\"") ||
      !contains(navigation, "\"GetChildren\"") ||
      !contains(navigation, "\"GetIndexInParent\"") ||
      !contains(navigation, "\"GetApplication\"") ||
      !contains(navigation, "DBUS_TYPE_OBJECT_PATH") ||
      !contains(navigation, "DBUS_TYPE_ARRAY") ||
      !contains(navigation, "\"(so)\"")) {
    return 4;
  }
  if (!contains(messages, "property name=\"Parent\" type=\"(so)\"") ||
      !contains(messages, "property name=\"ChildCount\" type=\"i\"") ||
      !contains(messages, "type=\"a(so)\"") ||
      !contains(messages, "wayland_atspi_navigation_reply_for(")) {
    return 5;
  }
  if (!contains(test_support, "WaylandAtspiDbusRecorder") ||
      !contains(test_support, "reply_object_reference(") ||
      !contains(test_support, "reply_object_references(") ||
      !contains(behavior, "GetChildAtIndex") ||
      !contains(behavior, "GetChildren") ||
      !contains(behavior, "GetIndexInParent") ||
      !contains(behavior, "GetApplication") ||
      !contains(behavior, "ChildCount")) {
    return 6;
  }
  if (!contains(xmake, "target(\"phase_g_wayland_atspi_tree_navigation_test\")") ||
      !contains(xmake,
                "target(\"phase_g_wayland_atspi_tree_navigation_structure_test\")") ||
      !contains(previous, "WaylandAtspiDbusRecorder")) {
    return 7;
  }
  if (line_count(object_header) > 50 || line_count(object) > 65 ||
      line_count(dbus_header) > 90 || line_count(dbus) > 180 ||
      line_count(messages) > 105 || line_count(navigation_header) > 30 ||
      line_count(navigation) > 220 || line_count(test_support) > 230 ||
      line_count(behavior) > 180) {
    return 8;
  }

  constexpr const char* completion =
      "Phase G Step 628 adds Linux AT-SPI Accessible tree navigation with "
      "Parent and ChildCount properties, ordered child lookup and enumeration, "
      "parent indexes, application roots, and standard D-Bus object references. "
      "Step 629 Linux AT-SPI roles and states production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 9;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 678 Phase G final closeout verification\"")) {
    return 10;
  }
  return 0;
}
