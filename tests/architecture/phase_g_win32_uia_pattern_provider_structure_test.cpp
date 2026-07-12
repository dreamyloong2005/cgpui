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
  const std::string event = read_source(
      "include/cgpui/core/event_accessibility.hpp");
  const std::string patterns = read_source(
      "include/cgpui/ui/accessibility.hpp");
  const std::string element = read_source("include/cgpui/ui/element_core.hpp");
  const std::string platform = read_source(
      "include/cgpui/platform/platform_accessibility.hpp");
  const std::string runtime = read_source(
      "src/ui/runtime_accessibility_actions.cpp");
  const std::string runtime_header = read_source(
      "src/ui/runtime_window_accessibility_internal.hpp");
  const std::string root_events = read_source("src/ui/runtime_events.cpp");
  const std::string window_events = read_source(
      "src/ui/runtime_event_windows.cpp");
  const std::string object = read_source(
      "src/platform/win32/win32_uia_provider_object_internal.hpp");
  const std::string provider = read_source(
      "src/platform/win32/win32_uia_provider.cpp");
  const std::string win32_actions = read_source(
      "src/platform/win32/win32_uia_actions.cpp");
  const std::string window_accessibility = read_source(
      "src/platform/win32/win32_window_accessibility.cpp");
  const std::string win32_patterns = read_source(
      "src/platform/win32/win32_uia_patterns.cpp");
  const std::string range_pattern = read_source(
      "src/platform/win32/win32_uia_range_pattern.cpp");
  const std::string pattern_properties_header = read_source(
      "src/platform/win32/win32_uia_pattern_properties_internal.hpp");
  const std::string pattern_properties = read_source(
      "src/platform/win32/win32_uia_pattern_properties.cpp");
  const std::string slider_accessibility = read_source(
      "src/ui/element_slider_accessibility.cpp");
  const std::string text_accessibility = read_source(
      "src/ui/element_text_accessibility.cpp");
  const std::string element_behavior = read_source(
      "tests/ui/accessibility_pattern_action_test.cpp");
  const std::string runtime_behavior = read_source(
      "tests/ui/accessibility_pattern_runtime_test.cpp");
  const std::string provider_behavior = read_source(
      "tests/platform/win32_uia_pattern_provider_test.cpp");
  const std::string core_headers = read_source(
      "tests/header_cleanliness/core_header_cleanliness.cpp");
  const std::string ui_headers = read_source(
      "tests/header_cleanliness/ui_header_cleanliness.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &event, &patterns, &element, &platform, &runtime, &runtime_header,
      &root_events, &window_events, &object, &provider, &win32_actions,
      &window_accessibility, &win32_patterns, &range_pattern,
      &pattern_properties_header,
      &pattern_properties,
      &slider_accessibility, &text_accessibility, &element_behavior,
      &runtime_behavior, &provider_behavior, &core_headers, &ui_headers,
      &xmake, &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(event, "enum class AccessibilityActionKind") ||
      !contains(event, "struct AccessibilityActionRequested") ||
      !contains(patterns, "struct AccessibilityPatternState") ||
      !contains(element, "accessibility_patterns()") ||
      !contains(element, "handle_accessibility_action(")) return 2;
  if (!contains(platform, "PlatformAccessibilityPatternState") ||
      !contains(runtime_header, "handle_accessibility_action_event(") ||
      !contains(runtime, "target_view_id == root_view_id_") ||
      !contains(runtime, "finish_event_dispatch(result)") ||
      !contains(runtime, "finish_event_dispatch_for_record(*record, result)") ||
      !contains(root_events, "handle_accessibility_action_event(event, root_view_id_)") ||
      !contains(window_events, "record->root_view_id, record")) return 3;
  if (!contains(object, "public IInvokeProvider") ||
      !contains(object, "public IValueProvider") ||
      !contains(object, "public IToggleProvider") ||
      !contains(object, "public IRangeValueProvider") ||
      !contains(provider,
                "set_win32_uia_pattern_availability_property(") ||
      !contains(win32_actions, "set_action_callback(") ||
      !contains(window_accessibility,
                "Win32Window::configure_accessibility_actions()") ||
      !contains(win32_patterns, "GetPatternProvider(") ||
      !contains(win32_patterns, "Win32UiaProvider::Invoke()") ||
      !contains(win32_patterns, "Win32UiaProvider::Toggle()") ||
      !contains(win32_patterns, "Win32UiaProvider::SetValue(LPCWSTR value)") ||
      !contains(range_pattern, "Win32UiaProvider::SetValue(double value)") ||
      !contains(pattern_properties_header,
                "set_win32_uia_pattern_availability_property(") ||
      !contains(pattern_properties,
                "UIA_IsInvokePatternAvailablePropertyId") ||
      !contains(slider_accessibility,
                "SliderElement::handle_accessibility_action(") ||
      !contains(text_accessibility,
                "TextInputElement::handle_accessibility_action(")) {
    return 4;
  }
  if (!contains(element_behavior, "AccessibilityActionKind::set_range_value") ||
      !contains(runtime_behavior, "EventKind::accessibility_action") ||
      !contains(runtime_behavior, "accessibility_update_count") ||
      !contains(provider_behavior, "UIA_InvokePatternId") ||
      !contains(provider_behavior, "UIA_RangeValuePatternId")) return 5;
  if (!contains(core_headers, "event_accessibility.hpp") ||
      !contains(ui_headers, "cgpui/ui/accessibility.hpp") ||
      !contains(xmake, "target(\"accessibility_pattern_runtime_test\")") ||
      !contains(xmake, "target(\"win32_uia_pattern_provider_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_pattern_provider_structure_test.cpp")) {
    return 6;
  }
  if (line_count(event) > 35 || line_count(patterns) > 35 ||
      line_count(runtime) > 70 || line_count(win32_patterns) > 130 ||
      line_count(pattern_properties) > 50 ||
      line_count(slider_accessibility) > 80 ||
      line_count(text_accessibility) > 40 ||
      line_count(element_behavior) > 90 || line_count(runtime_behavior) > 90 ||
      line_count(provider_behavior) > 150) return 7;

  constexpr const char* completion =
      "Phase G Step 621 adds production Win32 UIA Invoke, Value, Toggle, and "
      "RangeValue pattern providers routed through runtime accessibility "
      "actions to real elements. Step 622 Win32 UIA live event production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 654 SVG asset decode boundary production behavior\"")) return 9;
  return 0;
}
