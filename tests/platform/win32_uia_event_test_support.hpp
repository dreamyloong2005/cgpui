#pragma once

#include "win32_uia_events_internal.hpp"

#include <UIAutomationClient.h>
#include <OleAuto.h>

#include <string>
#include <vector>

namespace cgpui::test {

struct UiaPropertyCall {
  IRawElementProviderSimple* provider = nullptr;
  PROPERTYID property = 0;
  VARIANT old_value{};
  VARIANT new_value{};
};

struct UiaEventCall {
  IRawElementProviderSimple* provider = nullptr;
  EVENTID event = 0;
};

inline std::vector<UiaPropertyCall> uia_property_calls;
inline std::vector<UiaEventCall> uia_event_calls;
inline bool uia_clients_listening = true;
inline HRESULT uia_property_status = S_OK;
inline HRESULT uia_event_status = S_OK;

inline BOOL WINAPI test_uia_clients_are_listening() {
  return uia_clients_listening ? TRUE : FALSE;
}

inline HRESULT WINAPI test_uia_raise_property_changed(
    IRawElementProviderSimple* provider,
    PROPERTYID property,
    VARIANT old_value,
    VARIANT new_value) {
  UiaPropertyCall call{.provider = provider, .property = property};
  VariantCopy(&call.old_value, &old_value);
  VariantCopy(&call.new_value, &new_value);
  uia_property_calls.push_back(call);
  return uia_property_status;
}

inline HRESULT WINAPI test_uia_raise_automation_event(
    IRawElementProviderSimple* provider,
    EVENTID event) {
  uia_event_calls.push_back(UiaEventCall{.provider = provider, .event = event});
  return uia_event_status;
}

inline Win32UiaEventOperations test_uia_event_operations() {
  return Win32UiaEventOperations{
      .clients_are_listening = test_uia_clients_are_listening,
      .raise_property_changed = test_uia_raise_property_changed,
      .raise_automation_event = test_uia_raise_automation_event,
  };
}

inline bool uia_bstr_equals(const VARIANT& value, const wchar_t* expected) {
  return value.vt == VT_BSTR && value.bstrVal != nullptr &&
      std::wstring{value.bstrVal, SysStringLen(value.bstrVal)} == expected;
}

inline bool uia_bool_equals(const VARIANT& value, bool expected) {
  return value.vt == VT_BOOL &&
      (value.boolVal == VARIANT_TRUE) == expected;
}

inline void reset_uia_event_calls() {
  for (auto& call : uia_property_calls) {
    VariantClear(&call.old_value);
    VariantClear(&call.new_value);
  }
  uia_property_calls.clear();
  uia_event_calls.clear();
  uia_clients_listening = true;
  uia_property_status = S_OK;
  uia_event_status = S_OK;
}

} // namespace cgpui::test
