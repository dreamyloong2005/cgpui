#include "win32_internal.hpp"

#include <UIAutomationClient.h>
#include <UIAutomationCoreApi.h>

#include <atomic>
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

class Win32UiaProvider final : public IRawElementProviderSimple {
 public:
  Win32UiaProvider(HWND hwnd, Win32UiaProviderNode node)
      : hwnd_(hwnd), node_(std::move(node)) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
    if (object == nullptr) return E_POINTER;
    *object = nullptr;
    if (IsEqualIID(iid, IID_IUnknown) ||
        IsEqualIID(iid, IID_IRawElementProviderSimple)) {
      *object = static_cast<IRawElementProviderSimple*>(this);
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }

  ULONG STDMETHODCALLTYPE AddRef() override { return ++reference_count_; }
  ULONG STDMETHODCALLTYPE Release() override {
    const ULONG count = --reference_count_;
    if (count == 0) delete this;
    return count;
  }

  HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* options) override {
    if (options == nullptr) return E_POINTER;
    *options = ProviderOptions_ServerSideProvider;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetPatternProvider(
      PATTERNID,
      IUnknown** provider) override {
    if (provider == nullptr) return E_POINTER;
    *provider = nullptr;
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE GetPropertyValue(
      PROPERTYID property,
      VARIANT* value) override {
    if (value == nullptr) return E_POINTER;
    VariantInit(value);
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
      return set_bounding_rect(value, hwnd_, node_.bounds);
    }
    return S_OK;
  }

  HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(
      IRawElementProviderSimple** provider) override {
    if (provider == nullptr) return E_POINTER;
    *provider = nullptr;
    return hwnd_ == nullptr ? S_OK : UiaHostProviderFromHwnd(hwnd_, provider);
  }

  void set_hwnd(HWND hwnd) { hwnd_ = hwnd; }

 private:
  std::atomic<ULONG> reference_count_{1};
  HWND hwnd_ = nullptr;
  Win32UiaProviderNode node_;
};

} // namespace

IRawElementProviderSimple* create_win32_uia_provider(
    HWND hwnd,
    Win32UiaProviderNode node) {
  return new (std::nothrow) Win32UiaProvider(hwnd, std::move(node));
}

void set_win32_uia_provider_hwnd(
    IRawElementProviderSimple* provider,
    HWND hwnd) {
  if (provider != nullptr) {
    static_cast<Win32UiaProvider*>(provider)->set_hwnd(hwnd);
  }
}

} // namespace cgpui
