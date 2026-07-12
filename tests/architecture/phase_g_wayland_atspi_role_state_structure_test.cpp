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
  const std::string role_header = read_source(
      "src/platform/linux/wayland_atspi_role_state_internal.hpp");
  const std::string role = read_source(
      "src/platform/linux/wayland_atspi_role_state.cpp");
  const std::string messages = read_source(
      "src/platform/linux/wayland_atspi_dbus_messages.cpp");
  const std::string behavior = read_source(
      "tests/platform/wayland_atspi_role_state_test.cpp");
  const std::string previous = read_source(
      "tests/architecture/phase_g_wayland_atspi_tree_navigation_structure_test.cpp");
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
      &object_header, &object, &role_header, &role, &messages, &behavior,
      &previous, &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan,
      &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(object_header, "PlatformAccessibilityPatternState patterns") ||
      !contains(object, ".patterns = node.patterns")) {
    return 2;
  }
  if (!contains(role_header, "struct WaylandAtspiRole") ||
      !contains(role_header, "wayland_atspi_role_for(") ||
      !contains(role_header, "wayland_atspi_states_for(") ||
      !contains(role_header, "wayland_atspi_role_state_reply_for(")) {
    return 3;
  }
  if (!contains(role, "PlatformAccessibilityRole::button") ||
      !contains(role, "{43, \"push button\"}") ||
      !contains(role, "{7, \"check box\"}") ||
      !contains(role, "{79, \"entry\"}") ||
      !contains(role, "{51, \"slider\"}") ||
      !contains(role, "{67, \"unknown\"}")) {
    return 4;
  }
  if (!contains(role, "set_state(words, 25)") ||
      !contains(role, "set_state(words, 30)") ||
      !contains(role, "set_state(words, 8)") ||
      !contains(role, "set_state(words, 24)") ||
      !contains(role, "set_state(words, 11)") ||
      !contains(role, "set_state(words, 12)") ||
      !contains(role, "set_state(words, 41)") ||
      !contains(role, "set_state(words, 4)") ||
      !contains(role, "DBUS_TYPE_UINT32_AS_STRING")) {
    return 5;
  }
  if (!contains(messages, "GetRoleName") ||
      !contains(messages, "GetLocalizedRoleName") ||
      !contains(messages, "GetState") ||
      !contains(messages, "wayland_atspi_role_state_reply_for(")) {
    return 6;
  }
  if (!contains(behavior, "reply_uint32(recorder.sent_reply) != 43") ||
      !contains(behavior, "reply_uint32(recorder.sent_reply) != 7") ||
      !contains(behavior, "state_present(button_states, 12)") ||
      !contains(behavior, "state_present(checkbox_states, 41)") ||
      !contains(
          behavior,
          "wayland_atspi_role_for(PlatformAccessibilityRole::text_input).id != 79")) {
    return 7;
  }
  if (!contains(xmake, "target(\"phase_g_wayland_atspi_role_state_test\")") ||
      !contains(xmake,
                "target(\"phase_g_wayland_atspi_role_state_structure_test\")")) {
    return 8;
  }
  if (line_count(object_header) > 55 || line_count(object) > 65 ||
      line_count(role_header) > 40 || line_count(role) > 160 ||
      line_count(messages) > 115 || line_count(behavior) > 180) {
    return 9;
  }

  constexpr const char* completion =
      "Phase G Step 629 maps Linux accessibility nodes to standard AT-SPI "
      "roles, role names, and two-word state sets covering visibility, "
      "enablement, sensitivity, focus, checkability, checked state, and "
      "editability. Step 630 Linux AT-SPI text and value event production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 10;
  }
  if (!contains(
          ledger_json,
          "\"phase_f_current_handoff\": \"Step 630 Linux AT-SPI text and "
          "value event production behavior\"")) {
    return 11;
  }
  return 0;
}
