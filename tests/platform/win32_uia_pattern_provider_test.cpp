#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>
#include <UIAutomationClient.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"

#include <cmath>
#include <string>
#include <vector>

int main() {
  std::vector<cgpui::AccessibilityActionRequested> actions;
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.set_action_callback(
      [&](cgpui::AccessibilityActionRequested action) {
        actions.push_back(std::move(action));
      });
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 6,
      .nodes = {
          {.element_id = 1},
          {.element_id = 2,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::button,
           .patterns = {.invokable = true}},
          {.element_id = 3,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::text_input,
           .value = "before",
           .patterns = {.value_settable = true}},
          {.element_id = 4,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::checkbox,
           .patterns = {.toggled = true}},
          {.element_id = 5,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::slider,
           .patterns = {.range = cgpui::PlatformAccessibilityRangeValue{
                            .value = 25.0,
                            .minimum = 0.0,
                            .maximum = 100.0,
                            .small_change = 5.0,
                            .large_change = 20.0}}},
          {.element_id = 6,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::button,
           .patterns = {.invokable = true},
           .enabled = false},
      }});

  IInvokeProvider* invoke = nullptr;
  if (adapter.provider_for_element(2)->GetPatternProvider(
          UIA_InvokePatternId, reinterpret_cast<IUnknown**>(&invoke)) != S_OK ||
      invoke == nullptr || invoke->Invoke() != S_OK) return 1;
  invoke->Release();
  if (actions.size() != 1 ||
      actions.back().kind != cgpui::AccessibilityActionKind::invoke ||
      actions.back().element_id != 2) return 2;

  IValueProvider* value = nullptr;
  if (adapter.provider_for_element(3)->GetPatternProvider(
          UIA_ValuePatternId, reinterpret_cast<IUnknown**>(&value)) != S_OK ||
      value == nullptr) return 3;
  BSTR text = nullptr;
  BOOL read_only = TRUE;
  if (value->get_Value(&text) != S_OK || text == nullptr ||
      std::wstring{text, SysStringLen(text)} != L"before" ||
      value->get_IsReadOnly(&read_only) != S_OK || read_only != FALSE ||
      value->SetValue(L"after") != S_OK) return 4;
  SysFreeString(text);
  value->Release();
  if (actions.back().kind != cgpui::AccessibilityActionKind::set_value ||
      actions.back().element_id != 3 || actions.back().value != "after") return 5;

  IToggleProvider* toggle = nullptr;
  ToggleState toggle_state = ToggleState_Off;
  if (adapter.provider_for_element(4)->GetPatternProvider(
          UIA_TogglePatternId, reinterpret_cast<IUnknown**>(&toggle)) != S_OK ||
      toggle == nullptr || toggle->get_ToggleState(&toggle_state) != S_OK ||
      toggle_state != ToggleState_On || toggle->Toggle() != S_OK) return 6;
  toggle->Release();
  if (actions.back().kind != cgpui::AccessibilityActionKind::toggle ||
      actions.back().element_id != 4) return 7;

  IRangeValueProvider* range = nullptr;
  if (adapter.provider_for_element(5)->GetPatternProvider(
          UIA_RangeValuePatternId,
          reinterpret_cast<IUnknown**>(&range)) != S_OK ||
      range == nullptr) return 8;
  double current = 0.0;
  double minimum = 0.0;
  double maximum = 0.0;
  double small_change = 0.0;
  double large = 0.0;
  if (range->get_Value(&current) != S_OK ||
      range->get_Minimum(&minimum) != S_OK ||
      range->get_Maximum(&maximum) != S_OK ||
      range->get_SmallChange(&small_change) != S_OK ||
      range->get_LargeChange(&large) != S_OK ||
      std::abs(current - 25.0) > 0.001 || minimum != 0.0 || maximum != 100.0 ||
      small_change != 5.0 || large != 20.0 ||
      range->SetValue(101.0) != E_INVALIDARG ||
      range->SetValue(80.0) != S_OK) return 9;
  range->Release();
  if (actions.back().kind != cgpui::AccessibilityActionKind::set_range_value ||
      actions.back().element_id != 5 ||
      std::abs(actions.back().numeric_value - 80.0) > 0.001) return 10;

  IUnknown* pattern = reinterpret_cast<IUnknown*>(1);
  if (adapter.provider_for_element(1)->GetPatternProvider(
          UIA_InvokePatternId, &pattern) != S_OK || pattern != nullptr) return 11;
  VARIANT available{};
  if (adapter.provider_for_element(2)->GetPropertyValue(
          UIA_IsInvokePatternAvailablePropertyId, &available) != S_OK ||
      available.vt != VT_BOOL || available.boolVal != VARIANT_TRUE) return 12;
  IInvokeProvider* disabled = nullptr;
  if (adapter.provider_for_element(6)->GetPatternProvider(
          UIA_InvokePatternId,
          reinterpret_cast<IUnknown**>(&disabled)) != S_OK ||
      disabled->Invoke() != UIA_E_ELEMENTNOTENABLED) return 13;
  disabled->Release();
  return 0;
}
