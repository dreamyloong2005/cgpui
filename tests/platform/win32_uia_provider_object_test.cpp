#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationClient.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"

#include <cmath>
#include <string>

namespace {

bool bstr_equals(const VARIANT& value, const wchar_t* expected) {
  return value.vt == VT_BSTR && value.bstrVal != nullptr &&
      std::wstring{value.bstrVal, SysStringLen(value.bstrVal)} == expected;
}

bool bool_equals(const VARIANT& value, bool expected) {
  return value.vt == VT_BOOL &&
      (value.boolVal == VARIANT_TRUE) == expected;
}

bool bounding_rect_equals(
    const VARIANT& value,
    double x,
    double y,
    double width,
    double height) {
  if (value.vt != (VT_ARRAY | VT_R8) || value.parray == nullptr) return false;
  double* values = nullptr;
  if (SafeArrayAccessData(value.parray, reinterpret_cast<void**>(&values)) != S_OK) {
    return false;
  }
  const bool matches = std::abs(values[0] - x) < 0.001 &&
      std::abs(values[1] - y) < 0.001 &&
      std::abs(values[2] - width) < 0.001 &&
      std::abs(values[3] - height) < 0.001;
  SafeArrayUnaccessData(value.parray);
  return matches;
}

} // namespace

int main() {
  HWND hwnd = CreateWindowExW(
      0, L"STATIC", L"CGPUI UIA provider test", WS_OVERLAPPED,
      0, 0, 320, 200, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (hwnd == nullptr) return 1;

  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.attach(hwnd);
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 42,
      .node_count = 2,
      .focused_node_count = 1,
      .nodes = {
          cgpui::PlatformAccessibilityNodeUpdate{
              .element_id = 42,
              .role = cgpui::PlatformAccessibilityRole::generic,
              .name = "Root",
              .enabled = true,
              .focusable = false,
              .focused = false,
              .bounds = cgpui::Rect{
                  .origin = {.x = 10.0F, .y = 20.0F},
                  .size = {.width = 300.0F, .height = 160.0F}},
              .child_count = 1,
          },
          cgpui::PlatformAccessibilityNodeUpdate{
              .element_id = 77,
              .parent_element_id = 42,
              .role = cgpui::PlatformAccessibilityRole::text_input,
              .name = "Search",
              .text = "query",
              .value = "query",
              .enabled = true,
              .focusable = true,
              .focused = true,
              .bounds = cgpui::Rect{
                  .origin = {.x = 24.0F, .y = 48.0F},
                  .size = {.width = 180.0F, .height = 32.0F}},
          },
      },
  });

  IRawElementProviderSimple* root = adapter.root_provider();
  IRawElementProviderSimple* input = adapter.provider_for_element(77);
  if (root == nullptr || input == nullptr ||
      adapter.provider_for_element(999) != nullptr) {
    DestroyWindow(hwnd);
    return 2;
  }

  void* object = reinterpret_cast<void*>(1);
  if (input->QueryInterface(IID_IRawElementProviderSimple, &object) != S_OK ||
      object != input || input->Release() != 1) {
    DestroyWindow(hwnd);
    return 3;
  }
  object = reinterpret_cast<void*>(1);
  if (input->QueryInterface(IID_IClassFactory, &object) != E_NOINTERFACE ||
      object != nullptr || input->QueryInterface(IID_IUnknown, nullptr) != E_POINTER) {
    DestroyWindow(hwnd);
    return 4;
  }

  ProviderOptions options{};
  if (input->get_ProviderOptions(&options) != S_OK ||
      (options & ProviderOptions_ServerSideProvider) == 0) {
    DestroyWindow(hwnd);
    return 5;
  }
  IRawElementProviderSimple* host = nullptr;
  if (input->get_HostRawElementProvider(&host) != S_OK || host == nullptr) {
    DestroyWindow(hwnd);
    return 6;
  }
  host->Release();

  VARIANT value{};
  VariantInit(&value);
  if (input->GetPropertyValue(UIA_AutomationIdPropertyId, &value) != S_OK ||
      !bstr_equals(value, L"cgpui-77")) return 7;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_ControlTypePropertyId, &value) != S_OK ||
      value.vt != VT_I4 || value.lVal != UIA_EditControlTypeId) return 8;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_NamePropertyId, &value) != S_OK ||
      !bstr_equals(value, L"Search")) return 9;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_ValueValuePropertyId, &value) != S_OK ||
      !bstr_equals(value, L"query")) return 10;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_IsEnabledPropertyId, &value) != S_OK ||
      !bool_equals(value, true)) return 11;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_IsKeyboardFocusablePropertyId, &value) != S_OK ||
      !bool_equals(value, true)) return 12;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_HasKeyboardFocusPropertyId, &value) != S_OK ||
      !bool_equals(value, true)) return 13;
  VariantClear(&value);
  if (input->GetPropertyValue(UIA_BoundingRectanglePropertyId, &value) != S_OK ||
      [&] {
        POINT origin{24, 48};
        ClientToScreen(hwnd, &origin);
        return bounding_rect_equals(
            value, origin.x, origin.y, 180.0, 32.0);
      }() == false) return 14;
  VariantClear(&value);

  IUnknown* pattern = reinterpret_cast<IUnknown*>(1);
  if (input->GetPatternProvider(UIA_ValuePatternId, &pattern) != S_OK ||
      pattern != nullptr || input->GetPatternProvider(UIA_ValuePatternId, nullptr) != E_POINTER) {
    DestroyWindow(hwnd);
    return 15;
  }

  adapter.detach();
  DestroyWindow(hwnd);
  return input->get_HostRawElementProvider(&host) == S_OK && host == nullptr
      ? 0
      : 16;
}
