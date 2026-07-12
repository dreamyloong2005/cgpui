#include "win32_internal.hpp"
#include "win32_uia_pattern_properties_internal.hpp"
#include "win32_uia_provider_object_internal.hpp"

#include <UIAutomationClient.h>
#include <UIAutomationCoreApi.h>

#include <new>
#include <string>
#include <utility>
namespace cgpui {
namespace {

CONTROLTYPEID control_type_for(PlatformAccessibilityRole role) {
  switch (role) {
    case PlatformAccessibilityRole::label:
    case PlatformAccessibilityRole::text:
      return UIA_TextControlTypeId;
    case PlatformAccessibilityRole::button:
      return UIA_ButtonControlTypeId;
    case PlatformAccessibilityRole::text_input:
      return UIA_EditControlTypeId;
    case PlatformAccessibilityRole::image:
      return UIA_ImageControlTypeId;
    case PlatformAccessibilityRole::checkbox:
      return UIA_CheckBoxControlTypeId;
    case PlatformAccessibilityRole::radio:
      return UIA_RadioButtonControlTypeId;
    case PlatformAccessibilityRole::switch_control:
      return UIA_ButtonControlTypeId;
    case PlatformAccessibilityRole::slider:
      return UIA_SliderControlTypeId;
    case PlatformAccessibilityRole::list_item:
      return UIA_ListItemControlTypeId;
    case PlatformAccessibilityRole::menu_item:
      return UIA_MenuItemControlTypeId;
    case PlatformAccessibilityRole::generic:
      return UIA_PaneControlTypeId;
  }
  return UIA_PaneControlTypeId;
}

HRESULT set_bstr(VARIANT* value, const std::wstring& text) {
  value->vt = VT_BSTR;
  value->bstrVal = SysAllocStringLen(
      text.data(), static_cast<UINT>(text.size()));
  return value->bstrVal != nullptr || text.empty() ? S_OK : E_OUTOFMEMORY;
}

HRESULT set_bounding_rect(
    VARIANT* value,
    HWND hwnd,
    const std::optional<Rect>& bounds) {
  if (!bounds.has_value()) return S_OK;
  SAFEARRAY* array = SafeArrayCreateVector(VT_R8, 0, 4);
  if (array == nullptr) return E_OUTOFMEMORY;
  double* values = nullptr;
  if (SafeArrayAccessData(array, reinterpret_cast<void**>(&values)) != S_OK) {
    SafeArrayDestroy(array);
    return E_FAIL;
  }
  POINT origin{
      static_cast<LONG>(bounds->origin.x),
      static_cast<LONG>(bounds->origin.y)};
  if (hwnd != nullptr) ClientToScreen(hwnd, &origin);
  values[0] = static_cast<double>(origin.x);
  values[1] = static_cast<double>(origin.y);
  values[2] = static_cast<double>(bounds->size.width);
  values[3] = static_cast<double>(bounds->size.height);
  SafeArrayUnaccessData(array);
  value->vt = VT_ARRAY | VT_R8;
  value->parray = array;
  return S_OK;
}

} // namespace

Win32UiaProvider::Win32UiaProvider(
    Win32UiaProviderTreeHandle tree,
    Win32UiaProviderNode node,
    bool is_root)
    : tree_(std::move(tree)), node_(std::move(node)), is_root_(is_root) {
  register_win32_uia_fragment(
      tree_, node_.element_id, static_cast<IRawElementProviderFragment*>(this));
}

Win32UiaProvider::~Win32UiaProvider() {
  unregister_win32_uia_fragment(
      tree_, node_.element_id, static_cast<IRawElementProviderFragment*>(this));
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::QueryInterface(
    REFIID iid,
    void** object) {
  if (object == nullptr) return E_POINTER;
  *object = nullptr;
  if (IsEqualIID(iid, IID_IUnknown) ||
      IsEqualIID(iid, IID_IRawElementProviderSimple)) {
    *object = static_cast<IRawElementProviderSimple*>(this);
  } else if (IsEqualIID(iid, IID_IRawElementProviderFragment)) {
    *object = static_cast<IRawElementProviderFragment*>(this);
  } else if (is_root_ &&
             IsEqualIID(iid, IID_IRawElementProviderFragmentRoot)) {
    *object = static_cast<IRawElementProviderFragmentRoot*>(this);
  } else if (node_.patterns.invokable && IsEqualIID(iid, IID_IInvokeProvider)) {
    *object = static_cast<IInvokeProvider*>(this);
  } else if (node_.patterns.value_settable &&
             IsEqualIID(iid, IID_IValueProvider)) {
    *object = static_cast<IValueProvider*>(this);
  } else if (node_.patterns.toggled.has_value() &&
             IsEqualIID(iid, IID_IToggleProvider)) {
    *object = static_cast<IToggleProvider*>(this);
  } else if (node_.patterns.range.has_value() &&
             IsEqualIID(iid, IID_IRangeValueProvider)) {
    *object = static_cast<IRangeValueProvider*>(this);
  } else {
    return E_NOINTERFACE;
  }
  AddRef();
  return S_OK;
}

ULONG STDMETHODCALLTYPE Win32UiaProvider::AddRef() {
  return ++reference_count_;
}

ULONG STDMETHODCALLTYPE Win32UiaProvider::Release() {
  const ULONG count = --reference_count_;
  if (count == 0) delete this;
  return count;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_ProviderOptions(
    ProviderOptions* options) {
  if (options == nullptr) return E_POINTER;
  *options = ProviderOptions_ServerSideProvider;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::GetPropertyValue(
    PROPERTYID property,
    VARIANT* value) {
  if (value == nullptr) return E_POINTER;
  VariantInit(value);
  if (set_win32_uia_pattern_availability_property(node_, property, value)) {
    return S_OK;
  }
  if (property == UIA_AutomationIdPropertyId) {
    return set_bstr(value, L"cgpui-" + std::to_wstring(node_.element_id));
  }
  if (property == UIA_ControlTypePropertyId) {
    value->vt = VT_I4;
    value->lVal = control_type_for(node_.role);
  } else if (property == UIA_NamePropertyId) {
    return set_bstr(value, widen(node_.name));
  } else if (property == UIA_ValueValuePropertyId) {
    return set_bstr(value, widen(node_.value));
  } else if (property == UIA_IsEnabledPropertyId) {
    value->vt = VT_BOOL;
    value->boolVal = node_.enabled ? VARIANT_TRUE : VARIANT_FALSE;
  } else if (property == UIA_IsKeyboardFocusablePropertyId) {
    value->vt = VT_BOOL;
    value->boolVal = node_.focusable ? VARIANT_TRUE : VARIANT_FALSE;
  } else if (property == UIA_HasKeyboardFocusPropertyId) {
    value->vt = VT_BOOL;
    value->boolVal = node_.focused ? VARIANT_TRUE : VARIANT_FALSE;
  } else if (property == UIA_BoundingRectanglePropertyId) {
    return set_bounding_rect(
        value, win32_uia_provider_tree_hwnd(tree_), node_.bounds);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32UiaProvider::get_HostRawElementProvider(
    IRawElementProviderSimple** provider) {
  if (provider == nullptr) return E_POINTER;
  *provider = nullptr;
  const HWND hwnd = win32_uia_provider_tree_hwnd(tree_);
  return hwnd == nullptr ? S_OK : UiaHostProviderFromHwnd(hwnd, provider);
}

IRawElementProviderSimple* create_win32_uia_provider(
    Win32UiaProviderTreeHandle tree,
    Win32UiaProviderNode node,
    bool is_root) {
  return new (std::nothrow) Win32UiaProvider(
      std::move(tree), std::move(node), is_root);
}

} // namespace cgpui
