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
  const std::string manifests = read_source("tests/platform/mcpp.toml") +
      read_source("packages/platform-win32/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml");
  const std::string* required[]{
      &provider_header, &provider_object_header, &provider, &provider_state,
      &lifetime, &adapter_header, &adapter, &updates, &window,
      &application, &behavior, &manifests};
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
  if (!contains(manifests, "\"win32_uia_provider_object_test.cpp\"") ||
      !contains(manifests, "\"win32_uia_provider_object_test.cpp\"") ||
      !contains(manifests, "\"uiautomationcore.lib\"") ||
      !contains(manifests, "\"oleaut32.lib\"") ||
      !contains(manifests,
                "\"phase_g_win32_uia_provider_object_structure_test.cpp\"")) return 6;
  if (line_count(provider_header) > 60 ||
      line_count(provider_object_header) > 90 ||
      line_count(provider) > 190 ||
      line_count(adapter_header) > 80 || line_count(adapter) > 120 ||
      line_count(updates) > 80 ||
      line_count(window) > 130 || line_count(application) > 220 ||
      line_count(behavior) > 180) return 7;

  return 0;
}
