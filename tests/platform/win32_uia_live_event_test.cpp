#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>
#include <UIAutomationClient.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"
#include "win32_uia_events_internal.hpp"

#include <string>
#include <vector>

namespace {

struct PropertyCall {
  IRawElementProviderSimple* provider = nullptr;
  PROPERTYID property = 0;
  VARIANT old_value{};
  VARIANT new_value{};
};

struct EventCall {
  IRawElementProviderSimple* provider = nullptr;
  EVENTID event = 0;
};

std::vector<PropertyCall> property_calls;
std::vector<EventCall> event_calls;
bool clients_listening = true;
HRESULT property_status = S_OK;
HRESULT event_status = S_OK;

BOOL WINAPI test_clients_are_listening() {
  return clients_listening ? TRUE : FALSE;
}

HRESULT WINAPI test_raise_property_changed(
    IRawElementProviderSimple* provider,
    PROPERTYID property,
    VARIANT old_value,
    VARIANT new_value) {
  PropertyCall call{.provider = provider, .property = property};
  VariantCopy(&call.old_value, &old_value);
  VariantCopy(&call.new_value, &new_value);
  property_calls.push_back(call);
  return property_status;
}

HRESULT WINAPI test_raise_automation_event(
    IRawElementProviderSimple* provider,
    EVENTID event) {
  event_calls.push_back(EventCall{.provider = provider, .event = event});
  return event_status;
}

bool bstr_equals(const VARIANT& value, const wchar_t* expected) {
  return value.vt == VT_BSTR && value.bstrVal != nullptr &&
      std::wstring{value.bstrVal, SysStringLen(value.bstrVal)} == expected;
}

bool bool_equals(const VARIANT& value, bool expected) {
  return value.vt == VT_BOOL &&
      (value.boolVal == VARIANT_TRUE) == expected;
}

void clear_calls() {
  for (auto& call : property_calls) {
    VariantClear(&call.old_value);
    VariantClear(&call.new_value);
  }
  property_calls.clear();
  event_calls.clear();
}

} // namespace

int main() {
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.set_event_operations(cgpui::Win32UiaEventOperations{
      .clients_are_listening = test_clients_are_listening,
      .raise_property_changed = test_raise_property_changed,
      .raise_automation_event = test_raise_automation_event,
  });
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 3,
      .focused_node_count = 1,
      .nodes = {
          {.element_id = 1},
          {.element_id = 2,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::text_input,
           .text = "before",
           .value = "before",
           .focusable = true},
          {.element_id = 3,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::button,
           .focusable = true,
           .focused = true},
      },
  });
  if (adapter.last_event_publication().update_count != 0) return 1;

  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 3,
      .focused_node_count = 1,
      .nodes = {
          {.element_id = 1},
          {.element_id = 2,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::text_input,
           .text = "after",
           .value = "after",
           .focusable = true,
           .focused = true},
          {.element_id = 3,
           .parent_element_id = 1,
           .role = cgpui::PlatformAccessibilityRole::button,
           .focusable = true},
      },
      .live_updates = {
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::value_changed,
           .element_id = 2,
           .value = "after"},
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::text_changed,
           .element_id = 2,
           .text = "after"},
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::focus_changed,
           .element_id = 2,
           .focused = true},
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::focus_changed,
           .element_id = 3},
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::value_changed,
           .element_id = 999,
           .value = "missing"},
      },
  });

  const auto publication = adapter.last_event_publication();
  if (publication.update_count != 5 || publication.property_changed_count != 3 ||
      publication.automation_event_count != 2 ||
      publication.missing_provider_count != 1 || publication.failure_count != 0) {
    return 2;
  }
  if (property_calls.size() != 3 || event_calls.size() != 2) return 3;
  if (property_calls[0].provider != adapter.provider_for_element(2) ||
      property_calls[0].property != UIA_ValueValuePropertyId ||
      !bstr_equals(property_calls[0].old_value, L"before") ||
      !bstr_equals(property_calls[0].new_value, L"after")) return 4;
  if (property_calls[1].property != UIA_HasKeyboardFocusPropertyId ||
      !bool_equals(property_calls[1].old_value, false) ||
      !bool_equals(property_calls[1].new_value, true) ||
      property_calls[2].provider != adapter.provider_for_element(3) ||
      !bool_equals(property_calls[2].old_value, true) ||
      !bool_equals(property_calls[2].new_value, false)) return 5;
  if (event_calls[0].provider != adapter.provider_for_element(2) ||
      event_calls[0].event != UIA_Text_TextChangedEventId ||
      event_calls[1].provider != adapter.provider_for_element(2) ||
      event_calls[1].event != UIA_AutomationFocusChangedEventId) return 6;

  clear_calls();
  clients_listening = false;
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 1,
      .nodes = {{.element_id = 1, .value = "ignored"}},
      .live_updates = {
          {.kind = cgpui::PlatformAccessibilityLiveUpdateKind::value_changed,
           .element_id = 1,
           .value = "ignored"},
      },
  });
  const auto skipped = adapter.last_event_publication();
  const bool passed = skipped.update_count == 1 &&
      skipped.skipped_client_count == 1 && property_calls.empty() &&
      event_calls.empty();
  clear_calls();
  if (!passed) return 7;

  clients_listening = true;
  event_status = E_FAIL;
  adapter.update(cgpui::PlatformAccessibilityTreeUpdate{
      .root_element_id = 1,
      .node_count = 1,
      .nodes = {{.element_id = 1, .text = "failed"}},
      .live_updates = {{.kind =
          cgpui::PlatformAccessibilityLiveUpdateKind::text_changed,
          .element_id = 1,
          .text = "failed"}},
  });
  const auto failed = adapter.last_event_publication();
  clear_calls();
  return failed.failure_count == 1 && failed.last_status == E_FAIL &&
      failed.automation_event_count == 0 ? 0 : 8;
}
