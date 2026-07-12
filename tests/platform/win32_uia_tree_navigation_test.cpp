#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"

#include <cmath>

namespace {

IRawElementProviderFragment* fragment_for(IRawElementProviderSimple* provider) {
  IRawElementProviderFragment* fragment = nullptr;
  return provider != nullptr &&
          provider->QueryInterface(
              IID_IRawElementProviderFragment,
              reinterpret_cast<void**>(&fragment)) == S_OK
      ? fragment
      : nullptr;
}

bool same_navigation(
    IRawElementProviderFragment* provider,
    NavigateDirection direction,
    IRawElementProviderFragment* expected) {
  IRawElementProviderFragment* result = nullptr;
  const HRESULT status = provider->Navigate(direction, &result);
  const bool matches = status == S_OK && result == expected;
  if (result != nullptr) result->Release();
  return matches;
}

bool runtime_id_equals(
    IRawElementProviderFragment* provider,
    LONG expected_high,
    LONG expected_low) {
  SAFEARRAY* runtime_id = nullptr;
  if (provider->GetRuntimeId(&runtime_id) != S_OK || runtime_id == nullptr) {
    return false;
  }
  LONG* values = nullptr;
  const bool matches = SafeArrayGetDim(runtime_id) == 1 &&
      SafeArrayAccessData(
          runtime_id, reinterpret_cast<void**>(&values)) == S_OK &&
      values[0] == UiaAppendRuntimeId && values[1] == expected_high &&
      values[2] == expected_low;
  if (values != nullptr) SafeArrayUnaccessData(runtime_id);
  SafeArrayDestroy(runtime_id);
  return matches;
}

} // namespace

int main() {
  HWND hwnd = CreateWindowExW(
      0, L"STATIC", L"CGPUI UIA navigation test", WS_OVERLAPPED,
      30, 40, 360, 240, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
  if (hwnd == nullptr) return 1;

  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.attach(hwnd);
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 42,
      .node_count = 4,
      .focused_node_count = 1,
      .nodes = {
          {.element_id = 42,
           .role = cgpui::PlatformAccessibilityRole::generic,
           .name = "Root",
           .bounds = cgpui::Rect{{0.0F, 0.0F}, {320.0F, 200.0F}},
           .child_count = 2},
          {.element_id = 10,
           .parent_element_id = 42,
           .role = cgpui::PlatformAccessibilityRole::generic,
           .name = "First",
           .bounds = cgpui::Rect{{10.0F, 10.0F}, {140.0F, 100.0F}},
           .child_count = 1},
          {.element_id = 11,
           .parent_element_id = 10,
           .role = cgpui::PlatformAccessibilityRole::button,
           .name = "Focused",
           .focusable = true,
           .focused = true,
           .bounds = cgpui::Rect{{20.0F, 20.0F}, {80.0F, 40.0F}}},
          {.element_id = 20,
           .parent_element_id = 42,
           .role = cgpui::PlatformAccessibilityRole::label,
           .name = "Last",
           .bounds = cgpui::Rect{{170.0F, 10.0F}, {120.0F, 80.0F}}},
      }});

  IRawElementProviderSimple* root_simple = adapter.root_provider();
  IRawElementProviderFragment* root = fragment_for(root_simple);
  IRawElementProviderFragment* first = fragment_for(
      adapter.provider_for_element(10));
  IRawElementProviderFragment* focused = fragment_for(
      adapter.provider_for_element(11));
  IRawElementProviderFragment* last = fragment_for(
      adapter.provider_for_element(20));
  if (root == nullptr || first == nullptr || focused == nullptr || last == nullptr) {
    return 2;
  }

  IRawElementProviderFragmentRoot* root_interface = nullptr;
  if (root_simple->QueryInterface(
          IID_IRawElementProviderFragmentRoot,
          reinterpret_cast<void**>(&root_interface)) != S_OK ||
      root_interface == nullptr) return 3;
  void* identity = nullptr;
  if (root->QueryInterface(IID_IUnknown, &identity) != S_OK ||
      identity != root_simple) return 4;
  static_cast<IUnknown*>(identity)->Release();
  void* unsupported = reinterpret_cast<void*>(1);
  if (adapter.provider_for_element(10)->QueryInterface(
          IID_IRawElementProviderFragmentRoot, &unsupported) != E_NOINTERFACE ||
      unsupported != nullptr) return 5;

  if (!same_navigation(root, NavigateDirection_FirstChild, first) ||
      !same_navigation(root, NavigateDirection_LastChild, last) ||
      !same_navigation(root, NavigateDirection_Parent, nullptr) ||
      !same_navigation(first, NavigateDirection_Parent, root) ||
      !same_navigation(first, NavigateDirection_NextSibling, last) ||
      !same_navigation(first, NavigateDirection_PreviousSibling, nullptr) ||
      !same_navigation(first, NavigateDirection_FirstChild, focused) ||
      !same_navigation(last, NavigateDirection_PreviousSibling, first) ||
      !same_navigation(last, NavigateDirection_NextSibling, nullptr)) return 6;

  if (!runtime_id_equals(focused, 0, 11) ||
      focused->GetRuntimeId(nullptr) != E_POINTER) return 7;

  UiaRect bounds{};
  POINT origin{20, 20};
  ClientToScreen(hwnd, &origin);
  if (focused->get_BoundingRectangle(&bounds) != S_OK ||
      std::abs(bounds.left - origin.x) > 0.001 ||
      std::abs(bounds.top - origin.y) > 0.001 ||
      std::abs(bounds.width - 80.0) > 0.001 ||
      std::abs(bounds.height - 40.0) > 0.001) return 8;

  IRawElementProviderFragmentRoot* fragment_root = nullptr;
  if (focused->get_FragmentRoot(&fragment_root) != S_OK ||
      fragment_root != root_interface) return 9;
  fragment_root->Release();

  IRawElementProviderFragment* result = nullptr;
  if (root_interface->GetFocus(&result) != S_OK || result != focused) return 10;
  result->Release();
  POINT hit{30, 30};
  ClientToScreen(hwnd, &hit);
  if (root_interface->ElementProviderFromPoint(hit.x, hit.y, &result) != S_OK ||
      result != focused) return 11;
  result->Release();

  SAFEARRAY* embedded = reinterpret_cast<SAFEARRAY*>(1);
  if (focused->GetEmbeddedFragmentRoots(&embedded) != S_OK ||
      embedded != nullptr || focused->SetFocus() != UIA_E_NOTSUPPORTED) return 12;

  root_interface->Release();
  root->Release();
  first->Release();
  focused->Release();
  last->Release();
  adapter.detach();
  DestroyWindow(hwnd);
  return 0;
}
