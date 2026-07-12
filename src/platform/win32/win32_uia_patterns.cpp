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
  if (!snapshot().has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
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
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::invoke,
      .element_id = state->node.element_id,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::SetValue(LPCWSTR value) {
  if (value == nullptr) return E_INVALIDARG;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::set_value,
      .element_id = state->node.element_id,
      .value = utf8_from_utf16(value),
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Value(BSTR* result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  const std::wstring value = widen(state->node.value);
  *result = SysAllocStringLen(value.data(), static_cast<UINT>(value.size()));
  return *result != nullptr || value.empty() ? S_OK : E_OUTOFMEMORY;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_IsReadOnly(BOOL* result) {
  if (result == nullptr) return E_POINTER;
  *result = FALSE;
  if (!snapshot().has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::Toggle() {
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::toggle,
      .element_id = state->node.element_id,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_ToggleState(
    ToggleState* result) {
  if (result == nullptr) return E_POINTER;
  *result = ToggleState_Off;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.toggled.value_or(false) ? ToggleState_On
                                                         : ToggleState_Off;
  return S_OK;
}

} // namespace cgpui
