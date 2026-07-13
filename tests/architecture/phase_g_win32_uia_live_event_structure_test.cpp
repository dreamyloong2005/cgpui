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
      "src/platform/win32/win32_uia_events_internal.hpp");
  const std::string events = read_source(
      "src/platform/win32/win32_uia_events.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string lifetime = read_source(
      "src/platform/win32/win32_uia_lifetime.cpp");
  const std::string adapter_header = read_source(
      "src/platform/win32/win32_accessibility_internal.hpp");
  const std::string adapter = read_source(
      "src/platform/win32/win32_accessibility.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_uia_live_event_test.cpp");
  const std::string test_support = read_source(
      "tests/platform/win32_uia_event_test_support.hpp");
  const std::string source_guard = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &event_header, &events, &updates, &lifetime, &adapter_header, &adapter,
      &behavior,
      &test_support, &source_guard, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(event_header, "struct Win32UiaEventOperations") ||
      !contains(event_header, "struct Win32UiaEventPublication") ||
      !contains(event_header, "publish_win32_uia_live_updates(") ||
      !contains(event_header, "UiaClientsAreListening") ||
      !contains(event_header, "UiaRaiseAutomationPropertyChangedEvent") ||
      !contains(event_header, "UiaRaiseAutomationEvent")) return 2;
  if (!contains(events, "UIA_ValueValuePropertyId") ||
      !contains(events, "UIA_Text_TextChangedEventId") ||
      !contains(events, "UIA_HasKeyboardFocusPropertyId") ||
      !contains(events, "UIA_AutomationFocusChangedEventId") ||
      !contains(events, "skipped_client_count") ||
      !contains(events, "missing_provider_count") ||
      !contains(events, "failure_count")) return 3;
  if (!contains(updates, "previous_nodes") ||
      !contains(lifetime, "create_win32_uia_provider_tree(") ||
      !contains(updates, "publish_win32_uia_live_updates(") ||
      !contains(adapter_header, "set_event_operations(") ||
      !contains(adapter_header, "last_event_publication() const") ||
      !contains(adapter, "Win32UiaAccessibilityAdapter::set_event_operations(")) {
    return 4;
  }
  if (!contains(test_support, "test_uia_raise_property_changed(") ||
      !contains(test_support, "test_uia_raise_automation_event(") ||
      !contains(behavior, "UIA_Text_TextChangedEventId") ||
      !contains(behavior, "UIA_AutomationFocusChangedEventId") ||
      !contains(behavior, "skipped_client_count") ||
      !contains(source_guard, "win32_uia_events.cpp") ||
      !contains(source_guard, "win32_uia_updates.cpp")) return 5;
  if (!contains(xmake, "target(\"win32_uia_live_event_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_live_event_structure_test.cpp")) return 6;
  if (line_count(event_header) > 60 || line_count(events) > 160 ||
      line_count(updates) > 80 || line_count(adapter_header) > 80 ||
      line_count(adapter) > 120 || line_count(behavior) > 150 ||
      line_count(test_support) > 100 ||
      line_count(source_guard) > 1150) return 7;

  constexpr const char* completion =
      "Phase G Step 622 publishes production Win32 UIA value-property, "
      "text-change, and focus notifications from runtime accessibility live "
      "updates with listener, missing-provider, and HRESULT diagnostics. "
      "Step 623 Win32 UIA focus, value, and text change integration is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 669 Windows and Linux release build and packaging coverage\"")) return 9;
  return 0;
}
