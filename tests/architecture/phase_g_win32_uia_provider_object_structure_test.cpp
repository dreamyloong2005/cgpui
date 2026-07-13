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
  const std::string provider_header = read_source(
      "src/platform/win32/win32_uia_provider_internal.hpp");
  const std::string provider_object_header = read_source(
      "src/platform/win32/win32_uia_provider_object_internal.hpp");
  const std::string provider = read_source(
      "src/platform/win32/win32_uia_provider.cpp");
  const std::string provider_state = read_source(
      "src/platform/win32/win32_uia_provider_state.cpp");
  const std::string lifetime = read_source(
      "src/platform/win32/win32_uia_lifetime.cpp");
  const std::string adapter_header = read_source(
      "src/platform/win32/win32_accessibility_internal.hpp");
  const std::string adapter = read_source(
      "src/platform/win32/win32_accessibility.cpp");
  const std::string updates = read_source(
      "src/platform/win32/win32_uia_updates.cpp");
  const std::string window = read_source("src/platform/win32/win32_window.cpp");
  const std::string application = read_source(
      "src/platform/win32/win32_application.cpp");
  const std::string behavior = read_source(
      "tests/platform/win32_uia_provider_object_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{
      &provider_header, &provider_object_header, &provider, &provider_state,
      &lifetime, &adapter_header, &adapter, &updates, &window,
      &application, &behavior, &xmake, &roadmap, &ledger_md, &ledger_json,
      &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(provider_header, "struct Win32UiaProviderNode") ||
      !contains(provider_header, "create_win32_uia_provider(") ||
      !contains(provider_object_header, "class Win32UiaProvider final") ||
      !contains(provider_object_header, "public IRawElementProviderSimple") ||
      !contains(provider, "QueryInterface(") || !contains(provider, "AddRef()") ||
      !contains(provider, "Release()") ||
      !contains(provider, "ProviderOptions_ServerSideProvider") ||
      !contains(provider_state, "UiaHostProviderFromHwnd(")) return 2;
  if (!contains(provider, "UIA_AutomationIdPropertyId") ||
      !contains(provider, "UIA_ControlTypePropertyId") ||
      !contains(provider, "UIA_NamePropertyId") ||
      !contains(provider, "UIA_ValueValuePropertyId") ||
      !contains(provider, "UIA_BoundingRectanglePropertyId")) return 3;
  if (!contains(adapter_header, "root_provider() const") ||
      !contains(adapter_header, "provider_for_element(") ||
      !contains(lifetime, "create_win32_uia_provider(") ||
      !contains(adapter, "release_providers()") ||
      !contains(window, "uia_accessibility_.attach(hwnd)") ||
      !contains(window, "uia_accessibility_.detach()") ||
      contains(window, "class Win32UiaProvider") ||
      contains(application, "IRawElementProviderSimple")) return 4;
  if (!contains(behavior, "IID_IRawElementProviderSimple") ||
      !contains(behavior, "UIA_EditControlTypeId") ||
      !contains(behavior, "get_HostRawElementProvider")) return 5;
  if (!contains(xmake, "target(\"win32_uia_provider_object_test\")") ||
      !contains(xmake, "tests/platform/win32_uia_provider_object_test.cpp") ||
      !contains(xmake, "\"uiautomationcore\"") ||
      !contains(xmake, "\"oleaut32\"") ||
      !contains(xmake,
                "tests/architecture/phase_g_win32_uia_provider_object_structure_test.cpp")) return 6;
  if (line_count(provider_header) > 60 ||
      line_count(provider_object_header) > 90 ||
      line_count(provider) > 190 ||
      line_count(adapter_header) > 80 || line_count(adapter) > 120 ||
      line_count(updates) > 80 ||
      line_count(window) > 130 || line_count(application) > 220 ||
      line_count(behavior) > 180) return 7;

  constexpr const char* completion =
      "Phase G Step 619 adds production Win32 `IRawElementProviderSimple` "
      "objects with COM identity and reference counting, HWND host providers, "
      "stable automation ids, role control types, basic properties, and "
      "adapter element lookup. Step 620 Win32 UIA tree navigation production "
      "behavior is next.";
  const std::string* documents[]{
      &roadmap, &ledger_md, &ledger_json, &task_plan, &findings};
  for (const auto* document : documents) if (!contains(*document, completion)) return 8;
  if (!contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 658 official image/GIF examples and asset closeout\"")) return 9;
  return 0;
}
