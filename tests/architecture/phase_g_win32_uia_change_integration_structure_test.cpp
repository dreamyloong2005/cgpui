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
  const std::string runtime_updates = read_source(
      "src/ui/ui_accessibility_internal.hpp");
  const std::string runtime_window = read_source(
      "src/ui/runtime_window_accessibility.cpp");
  const std::string events = read_source(
      "src/platform/win32/win32_uia_events.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string runtime_support = read_source(
      "tests/ui/window_runtime_test_support.hpp");
  const std::string event_support = read_source(
      "tests/platform/win32_uia_event_test_support.hpp");
  const std::string behavior = read_source(
      "tests/platform/win32_uia_runtime_change_integration_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &runtime_updates, &runtime_window, &events, &updates, &runtime_support,
      &event_support, &behavior, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(runtime_updates, "append_platform_accessibility_live_updates(") ||
      !contains(runtime_updates, "PlatformAccessibilityLiveUpdateKind::value_changed") ||
      !contains(runtime_updates, "PlatformAccessibilityLiveUpdateKind::text_changed") ||
      !contains(runtime_updates, "PlatformAccessibilityLiveUpdateKind::focus_changed") ||
      !contains(runtime_window, "record.window->update_accessibility_tree(")) {
    return 2;
  }
  if (!contains(runtime_support, "accessibility_update_callback") ||
      !contains(event_support, "test_uia_event_operations()") ||
      !contains(behavior, "request_keyboard_focus(integration_input_id)") ||
      !contains(behavior, "insert_text(\"!\")") ||
      !contains(behavior, "UIA_HasKeyboardFocusPropertyId") ||
      !contains(behavior, "UIA_ValueValuePropertyId") ||
      !contains(behavior, "UIA_AutomationFocusChangedEventId") ||
      !contains(behavior, "UIA_Text_TextChangedEventId")) return 3;
  if (!contains(updates, "publish_win32_uia_live_updates(") ||
      !contains(events, "UIA_ValueValuePropertyId") ||
      !contains(events, "UIA_Text_TextChangedEventId") ||
      !contains(events, "UIA_AutomationFocusChangedEventId")) return 4;
  if (!contains(xmake,
                "target(\"win32_uia_runtime_change_integration_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_change_integration_structure_test.cpp")) {
    return 5;
  }
  if (line_count(events) > 160 || line_count(updates) > 80 ||
      line_count(runtime_support) > 1800 || line_count(event_support) > 100 ||
      line_count(behavior) > 100) return 6;

  constexpr const char* completion =
      "Phase G Step 623 integrates runtime-generated Win32 UIA focus, value, "
      "and text changes end to end through the platform window and adapter "
      "event publisher. Step 624 Win32 UIA provider lifetime production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 7;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 670 examples and smoke test matrix coverage\"")) return 8;
  return 0;
}
