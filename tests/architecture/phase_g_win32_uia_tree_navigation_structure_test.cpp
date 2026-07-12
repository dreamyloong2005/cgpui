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
      "src/platform/win32/win32_uia_provider_object_internal.hpp");
  const std::string navigation_header = read_source(
      "src/platform/win32/win32_uia_navigation_internal.hpp");
  const std::string navigation = read_source(
      "src/platform/win32/win32_uia_navigation.cpp");
  const std::string tree_state_header = read_source(
      "src/platform/win32/win32_uia_tree_state_internal.hpp");
  const std::string lifetime = read_source(
      "src/platform/win32/win32_uia_lifetime.cpp");
  const std::string fragment = read_source(
      "src/platform/win32/win32_uia_fragment.cpp");
  const std::string adapter_header = read_source(
      "src/platform/win32/win32_accessibility_internal.hpp");
  const std::string adapter = read_source(
      "src/platform/win32/win32_accessibility.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string window = read_source(
      "src/platform/win32/win32_window_accessibility.cpp");
  const std::string proc_header = read_source(
      "src/platform/win32/win32_window_proc_accessibility_internal.hpp");
  const std::string proc = read_source(
      "src/platform/win32/win32_window_proc_accessibility.cpp");
  const std::string main_proc = read_source(
      "src/platform/win32/win32_window_proc.cpp");
  const std::string navigation_behavior = read_source(
      "tests/platform/win32_uia_tree_navigation_test.cpp");
  const std::string message_behavior = read_source(
      "tests/platform/win32_uia_get_object_message_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &object_header, &navigation_header, &navigation, &tree_state_header,
      &lifetime, &fragment,
      &adapter_header, &adapter, &updates, &window, &proc_header, &proc,
      &main_proc,
      &navigation_behavior, &message_behavior, &xmake, &roadmap, &ledger_md,
      &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(object_header, "public IRawElementProviderFragment") ||
      !contains(object_header, "public IRawElementProviderFragmentRoot") ||
      !contains(fragment, "Win32UiaProvider::Navigate(") ||
      !contains(fragment, "Win32UiaProvider::GetRuntimeId(") ||
      !contains(fragment, "Win32UiaProvider::get_FragmentRoot(") ||
      !contains(fragment, "Win32UiaProvider::ElementProviderFromPoint(") ||
      !contains(fragment, "Win32UiaProvider::GetFocus(")) return 2;
  if (!contains(navigation_header, "navigate_win32_uia_fragment(") ||
      !contains(tree_state_header, "class Win32UiaProviderTree") ||
      !contains(navigation, "NavigateDirection_Parent") ||
      !contains(navigation, "NavigateDirection_FirstChild") ||
      !contains(navigation, "NavigateDirection_NextSibling") ||
      !contains(navigation, "get_win32_uia_provider_from_point(") ||
      !contains(navigation, "get_win32_uia_focus(")) return 3;
  if (!contains(adapter_header, "handle_get_object(") ||
      !contains(lifetime, "create_win32_uia_provider_tree(") ||
      !contains(adapter, "UiaReturnRawElementProvider(") ||
      !contains(window, "Win32Window::accessibility_object(") ||
      !contains(proc_header, "class Win32AccessibilityMessageTarget") ||
      !contains(proc, "WM_GETOBJECT") ||
      !contains(main_proc, "win32_window_proc_handle_accessibility(") ||
      contains(main_proc, "WM_GETOBJECT")) return 4;
  if (!contains(navigation_behavior, "NavigateDirection_FirstChild") ||
      !contains(navigation_behavior, "GetRuntimeId(") ||
      !contains(navigation_behavior, "ElementProviderFromPoint(") ||
      !contains(message_behavior, "UiaRootObjectId") ||
      !contains(message_behavior, "handle_get_object(")) return 5;
  if (!contains(xmake, "target(\"win32_uia_tree_navigation_test\")") ||
      !contains(xmake, "target(\"win32_uia_get_object_message_test\")") ||
      !contains(xmake,
                "phase_g_win32_uia_tree_navigation_structure_test.cpp")) {
    return 6;
  }
  if (line_count(object_header) > 90 || line_count(navigation_header) > 70 ||
      line_count(navigation) > 230 || line_count(fragment) > 100 ||
      line_count(adapter_header) > 80 || line_count(adapter) > 140 ||
      line_count(updates) > 80 ||
      line_count(window) > 50 || line_count(proc_header) > 45 ||
      line_count(proc) > 35 || line_count(main_proc) > 80 ||
      line_count(navigation_behavior) > 190 ||
      line_count(message_behavior) > 100) return 7;

  constexpr const char* completion =
      "Phase G Step 620 adds production Win32 UIA fragment/root tree "
      "navigation with parent, child, and sibling traversal, runtime ids, "
      "bounds, focus and point lookup, plus focused `WM_GETOBJECT` routing. "
      "Step 621 Win32 UIA pattern provider production behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 8;
  }
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 642 async runtime production closeout audit\"")) return 9;
  return 0;
}
