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
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &runtime_updates, &runtime_window, &events, &updates, &runtime_support,
      &event_support, &behavior, &manifests};
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
  if (!contains(manifests,
                "\"win32_uia_runtime_change_integration_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_win32_uia_change_integration_structure_test.cpp\"")) {
    return 5;
  }
  if (line_count(events) > 160 || line_count(updates) > 80 ||
      line_count(runtime_support) > 1800 || line_count(event_support) > 100 ||
      line_count(behavior) > 100) return 6;

  return 0;
}
