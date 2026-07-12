#include "win32_internal.hpp"
#include "win32_uia_provider_object_internal.hpp"

#include <UIAutomationClient.h>
#include <OleAuto.h>

#include <string>

namespace cgpui {

HRESULT STDMETHODCALLTYPE Win32UiaProvider::GetPatternProvider(
    PATTERNID pattern,
    IUnknown** provider) {
  if (provider == nullptr) return E_POINTER;
  *provider = nullptr;
  const IID* iid = nullptr;
  if (pattern == UIA_InvokePatternId) {
    iid = &IID_IInvokeProvider;
  } else if (pattern == UIA_ValuePatternId) {
    iid = &IID_IValueProvider;
  } else if (pattern == UIA_TogglePatternId) {
    iid = &IID_IToggleProvider;
  } else if (pattern == UIA_RangeValuePatternId) {
    iid = &IID_IRangeValueProvider;
  }
  if (iid == nullptr) return S_OK;
  const HRESULT status = QueryInterface(*iid, reinterpret_cast<void**>(provider));
  return status == E_NOINTERFACE ? S_OK : status;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::Invoke() {
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::invoke,
      .element_id = node_.element_id,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::SetValue(LPCWSTR value) {
  if (value == nullptr) return E_INVALIDARG;
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::set_value,
      .element_id = node_.element_id,
      .value = utf8_from_utf16(value),
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Value(BSTR* result) {
  if (result == nullptr) return E_POINTER;
  const std::wstring value = widen(node_.value);
  *result = SysAllocStringLen(value.data(), static_cast<UINT>(value.size()));
  return *result != nullptr || value.empty() ? S_OK : E_OUTOFMEMORY;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_IsReadOnly(BOOL* result) {
  if (result == nullptr) return E_POINTER;
  *result = FALSE;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::Toggle() {
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::toggle,
      .element_id = node_.element_id,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_ToggleState(
    ToggleState* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.toggled.value_or(false) ? ToggleState_On
                                                   : ToggleState_Off;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::SetValue(double value) {
  if (!node_.patterns.range.has_value() ||
      value < node_.patterns.range->minimum ||
      value > node_.patterns.range->maximum) {
    return E_INVALIDARG;
  }
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::set_range_value,
      .element_id = node_.element_id,
      .numeric_value = value,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Value(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.range.has_value() ? node_.patterns.range->value : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Maximum(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.range.has_value() ? node_.patterns.range->maximum
                                             : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Minimum(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.range.has_value() ? node_.patterns.range->minimum
                                             : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_LargeChange(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.range.has_value()
      ? node_.patterns.range->large_change
      : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_SmallChange(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = node_.patterns.range.has_value()
      ? node_.patterns.range->small_change
      : 0.0;
  return S_OK;
}

} // namespace cgpui
