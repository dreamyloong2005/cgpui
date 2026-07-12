#include "win32_uia_provider_object_internal.hpp"

#include <UIAutomationCoreApi.h>

#include <cstdint>

namespace cgpui {

HRESULT STDMETHODCALLTYPE Win32UiaProvider::Navigate(
    NavigateDirection direction,
    IRawElementProviderFragment** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  const auto state = snapshot();
  return state.has_value()
      ? navigate_win32_uia_fragment(
            tree_, state->node.element_id, direction, result)
      : UIA_E_ELEMENTNOTAVAILABLE;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::GetRuntimeId(SAFEARRAY** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = SafeArrayCreateVector(VT_I4, 0, 3);
  if (*result == nullptr) return E_OUTOFMEMORY;
  LONG* values = nullptr;
  if (SafeArrayAccessData(*result, reinterpret_cast<void**>(&values)) != S_OK) {
    SafeArrayDestroy(*result);
    *result = nullptr;
    return E_FAIL;
  }
  values[0] = UiaAppendRuntimeId;
  values[1] = static_cast<LONG>(state->node.element_id >> 32U);
  values[2] = static_cast<LONG>(state->node.element_id & 0xffffffffU);
  SafeArrayUnaccessData(*result);
  return S_OK;
}
HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_BoundingRectangle(
    UiaRect* result) {
  if (result == nullptr) return E_POINTER;
  *result = UiaRect{};
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  if (!state->node.bounds.has_value()) return S_OK;
  POINT origin{
      static_cast<LONG>(state->node.bounds->origin.x),
      static_cast<LONG>(state->node.bounds->origin.y)};
  const HWND hwnd = win32_uia_provider_tree_hwnd(tree_);
  if (hwnd != nullptr) ClientToScreen(hwnd, &origin);
  result->left = origin.x;
  result->top = origin.y;
  result->width = state->node.bounds->size.width;
  result->height = state->node.bounds->size.height;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::GetEmbeddedFragmentRoots(
    SAFEARRAY** result) {
  if (result == nullptr) return E_POINTER;
  *result = nullptr;
  return snapshot().has_value() ? S_OK : UIA_E_ELEMENTNOTAVAILABLE;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::SetFocus() {
  return snapshot().has_value() ? UIA_E_NOTSUPPORTED
                                : UIA_E_ELEMENTNOTAVAILABLE;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_FragmentRoot(
    IRawElementProviderFragmentRoot** result) {
  if (!snapshot().has_value()) {
    if (result != nullptr) *result = nullptr;
    return result == nullptr ? E_POINTER : UIA_E_ELEMENTNOTAVAILABLE;
  }
  return get_win32_uia_fragment_root(tree_, result);
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::ElementProviderFromPoint(
    double x,
    double y,
    IRawElementProviderFragment** result) {
  if (!snapshot().has_value()) {
    if (result != nullptr) *result = nullptr;
    return result == nullptr ? E_POINTER : UIA_E_ELEMENTNOTAVAILABLE;
  }
  return get_win32_uia_provider_from_point(tree_, x, y, result);
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::GetFocus(
    IRawElementProviderFragment** result) {
  if (!snapshot().has_value()) {
    if (result != nullptr) *result = nullptr;
    return result == nullptr ? E_POINTER : UIA_E_ELEMENTNOTAVAILABLE;
  }
  return get_win32_uia_focus(tree_, result);
}

} // namespace cgpui
