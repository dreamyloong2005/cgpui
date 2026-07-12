#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>
#include <UIAutomationClient.h>
#include <OleAuto.h>

#include "win32_accessibility_internal.hpp"
#include "win32_uia_event_test_support.hpp"
#include "win32_uia_events_internal.hpp"

int main() {
  using namespace cgpui::test;
  reset_uia_event_calls();
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.set_event_operations(test_uia_event_operations());
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
  if (uia_property_calls.size() != 3 || uia_event_calls.size() != 2) return 3;
  if (uia_property_calls[0].provider != adapter.provider_for_element(2) ||
      uia_property_calls[0].property != UIA_ValueValuePropertyId ||
      !uia_bstr_equals(uia_property_calls[0].old_value, L"before") ||
      !uia_bstr_equals(uia_property_calls[0].new_value, L"after")) return 4;
  if (uia_property_calls[1].property != UIA_HasKeyboardFocusPropertyId ||
      !uia_bool_equals(uia_property_calls[1].old_value, false) ||
      !uia_bool_equals(uia_property_calls[1].new_value, true) ||
      uia_property_calls[2].provider != adapter.provider_for_element(3) ||
      !uia_bool_equals(uia_property_calls[2].old_value, true) ||
      !uia_bool_equals(uia_property_calls[2].new_value, false)) return 5;
  if (uia_event_calls[0].provider != adapter.provider_for_element(2) ||
      uia_event_calls[0].event != UIA_Text_TextChangedEventId ||
      uia_event_calls[1].provider != adapter.provider_for_element(2) ||
      uia_event_calls[1].event != UIA_AutomationFocusChangedEventId) return 6;

  reset_uia_event_calls();
  uia_clients_listening = false;
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
      skipped.skipped_client_count == 1 && uia_property_calls.empty() &&
      uia_event_calls.empty();
  reset_uia_event_calls();
  if (!passed) return 7;

  uia_clients_listening = true;
  uia_event_status = E_FAIL;
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
  reset_uia_event_calls();
  return failed.failure_count == 1 && failed.last_status == E_FAIL &&
      failed.automation_event_count == 0 ? 0 : 8;
}
