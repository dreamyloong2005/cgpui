#include "win32_internal.hpp"
#include "win32_uia_provider_object_internal.hpp"

namespace cgpui {

HRESULT STDMETHODCALLTYPE Win32UiaProvider::SetValue(double value) {
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  if (!state->node.patterns.range.has_value() ||
      value < state->node.patterns.range->minimum ||
      value > state->node.patterns.range->maximum) {
    return E_INVALIDARG;
  }
  return request_win32_uia_action(tree_, AccessibilityActionRequested{
      .kind = AccessibilityActionKind::set_range_value,
      .element_id = state->node.element_id,
      .numeric_value = value,
  });
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Value(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = 0.0;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.range.has_value()
      ? state->node.patterns.range->value
      : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Maximum(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = 0.0;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.range.has_value()
      ? state->node.patterns.range->maximum
      : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_Minimum(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = 0.0;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.range.has_value()
      ? state->node.patterns.range->minimum
      : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_LargeChange(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = 0.0;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.range.has_value()
      ? state->node.patterns.range->large_change
      : 0.0;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_SmallChange(double* result) {
  if (result == nullptr) return E_POINTER;
  *result = 0.0;
  const auto state = snapshot();
  if (!state.has_value()) return UIA_E_ELEMENTNOTAVAILABLE;
  *result = state->node.patterns.range.has_value()
      ? state->node.patterns.range->small_change
      : 0.0;
  return S_OK;
}

} // namespace cgpui
