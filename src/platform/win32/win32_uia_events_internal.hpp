#pragma once

#include "win32_uia_provider_internal.hpp"

#include <UIAutomationCoreApi.h>

#include <cstddef>
#include <span>

namespace cgpui {

struct Win32UiaEventOperations {
  BOOL (WINAPI* clients_are_listening)() = UiaClientsAreListening;
  HRESULT (WINAPI* raise_property_changed)(
      IRawElementProviderSimple*, PROPERTYID, VARIANT, VARIANT) =
      UiaRaiseAutomationPropertyChangedEvent;
  HRESULT (WINAPI* raise_automation_event)(
      IRawElementProviderSimple*, EVENTID) = UiaRaiseAutomationEvent;
};

struct Win32UiaEventPublication {
  std::size_t update_count = 0;
  std::size_t property_changed_count = 0;
  std::size_t automation_event_count = 0;
  std::size_t skipped_client_count = 0;
  std::size_t missing_provider_count = 0;
  std::size_t failure_count = 0;
  HRESULT last_status = S_OK;
};

Win32UiaEventPublication publish_win32_uia_live_updates(
    std::span<const PlatformAccessibilityLiveUpdate> updates,
    std::span<const Win32UiaProviderNode> previous_nodes,
    std::span<const Win32UiaProviderNode> current_nodes,
    std::span<IRawElementProviderSimple* const> providers,
    const Win32UiaEventOperations& operations);

} // namespace cgpui
