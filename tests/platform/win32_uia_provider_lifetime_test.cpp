#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationClient.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>

#include "win32_accessibility_internal.hpp"

#include <string>

namespace {

cgpui::PlatformAccessibilityTreeUpdate update_with_value(
    std::string value,
    bool include_input = true) {
  cgpui::PlatformAccessibilityTreeUpdate update{
      .root_element_id = 1,
      .node_count = include_input ? 2U : 1U,
      .nodes = {{.element_id = 1,
                 .role = cgpui::PlatformAccessibilityRole::generic,
                 .name = "Root",
                 .child_count = include_input ? 1U : 0U}}};
  if (include_input) {
    update.nodes.push_back({
        .element_id = 2,
        .parent_element_id = 1,
        .role = cgpui::PlatformAccessibilityRole::text_input,
        .name = "Input",
        .value = std::move(value),
        .patterns = {.value_settable = true},
        .enabled = true,
        .focusable = true,
    });
  }
  return update;
}

bool property_equals(
    IRawElementProviderSimple* provider,
    const wchar_t* expected) {
  VARIANT value{};
  VariantInit(&value);
  const HRESULT status = provider->GetPropertyValue(
      UIA_ValueValuePropertyId, &value);
  const bool matches = status == S_OK && value.vt == VT_BSTR &&
      value.bstrVal != nullptr &&
      std::wstring{value.bstrVal, SysStringLen(value.bstrVal)} == expected;
  VariantClear(&value);
  return matches;
}

} // namespace

int main() {
  IRawElementProviderSimple* retained_after_destroy = nullptr;
  {
    cgpui::Win32UiaAccessibilityAdapter adapter;
    adapter.update(update_with_value("first"));
    IRawElementProviderSimple* root = adapter.root_provider();
    IRawElementProviderSimple* input = adapter.provider_for_element(2);
    if (root == nullptr || input == nullptr || !property_equals(input, L"first")) {
      return 1;
    }
    input->AddRef();

    adapter.update(update_with_value("second"));
    if (adapter.root_provider() != root ||
        adapter.provider_for_element(2) != input ||
        !property_equals(input, L"second")) return 2;

    IValueProvider* value = nullptr;
    if (input->GetPatternProvider(
            UIA_ValuePatternId,
            reinterpret_cast<IUnknown**>(&value)) != S_OK ||
        value == nullptr) return 3;

    adapter.update(update_with_value({}, false));
    VARIANT unavailable{};
    VariantInit(&unavailable);
    IUnknown* pattern = reinterpret_cast<IUnknown*>(1);
    IRawElementProviderFragment* fragment = nullptr;
    IRawElementProviderFragment* navigation_result = nullptr;
    BSTR stale_value = nullptr;
    if (adapter.provider_for_element(2) != nullptr ||
        input->GetPropertyValue(UIA_NamePropertyId, &unavailable) !=
            UIA_E_ELEMENTNOTAVAILABLE ||
        input->GetPatternProvider(UIA_ValuePatternId, &pattern) !=
            UIA_E_ELEMENTNOTAVAILABLE ||
        pattern != nullptr || value->get_Value(&stale_value) !=
            UIA_E_ELEMENTNOTAVAILABLE ||
        stale_value != nullptr ||
        input->QueryInterface(
            IID_IRawElementProviderFragment,
            reinterpret_cast<void**>(&fragment)) != S_OK ||
        fragment->Navigate(NavigateDirection_Parent, &navigation_result) !=
            UIA_E_ELEMENTNOTAVAILABLE || navigation_result != nullptr) return 4;
    fragment->Release();
    value->Release();

    adapter.update(update_with_value("third"));
    IRawElementProviderSimple* replacement = adapter.provider_for_element(2);
    if (replacement == nullptr || replacement == input ||
        !property_equals(replacement, L"third") ||
        input->GetPropertyValue(UIA_NamePropertyId, &unavailable) !=
            UIA_E_ELEMENTNOTAVAILABLE) return 5;
    input->Release();

    retained_after_destroy = replacement;
    retained_after_destroy->AddRef();
  }

  VARIANT unavailable{};
  VariantInit(&unavailable);
  const bool destroyed_is_unavailable =
      retained_after_destroy->GetPropertyValue(
          UIA_NamePropertyId, &unavailable) == UIA_E_ELEMENTNOTAVAILABLE;
  retained_after_destroy->Release();
  return destroyed_is_unavailable ? 0 : 6;
}
