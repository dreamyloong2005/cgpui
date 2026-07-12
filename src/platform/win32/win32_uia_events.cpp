#include "win32_uia_events_internal.hpp"

#include "win32_internal.hpp"

#include <UIAutomationClient.h>

#include <algorithm>

namespace cgpui {
namespace {

const Win32UiaProviderNode* find_node(
    std::span<const Win32UiaProviderNode> nodes,
    std::uint64_t element_id) {
  const auto node = std::find_if(
      nodes.begin(), nodes.end(),
      [element_id](const Win32UiaProviderNode& candidate) {
        return candidate.element_id == element_id;
      });
  return node == nodes.end() ? nullptr : &*node;
}

IRawElementProviderSimple* find_provider(
    std::span<const Win32UiaProviderNode> nodes,
    std::span<IRawElementProviderSimple* const> providers,
    std::uint64_t element_id) {
  for (std::size_t index = 0; index < nodes.size(); ++index) {
    if (nodes[index].element_id == element_id && index < providers.size()) {
      return providers[index];
    }
  }
  return nullptr;
}

void record_status(
    Win32UiaEventPublication& publication,
    HRESULT status,
    std::size_t& success_count) {
  publication.last_status = status;
  if (SUCCEEDED(status)) {
    success_count += 1;
  } else {
    publication.failure_count += 1;
  }
}

VARIANT bstr_variant(const std::string& value) {
  VARIANT result{};
  result.vt = VT_BSTR;
  const std::wstring wide = widen(value);
  result.bstrVal = SysAllocStringLen(wide.data(), static_cast<UINT>(wide.size()));
  return result;
}

VARIANT bool_variant(bool value) {
  VARIANT result{};
  result.vt = VT_BOOL;
  result.boolVal = value ? VARIANT_TRUE : VARIANT_FALSE;
  return result;
}

} // namespace

Win32UiaEventPublication publish_win32_uia_live_updates(
    std::span<const PlatformAccessibilityLiveUpdate> updates,
    std::span<const Win32UiaProviderNode> previous_nodes,
    std::span<const Win32UiaProviderNode> current_nodes,
    std::span<IRawElementProviderSimple* const> providers,
    const Win32UiaEventOperations& operations) {
  Win32UiaEventPublication publication{.update_count = updates.size()};
  if (updates.empty()) return publication;
  if (operations.clients_are_listening == nullptr ||
      !operations.clients_are_listening()) {
    publication.skipped_client_count = updates.size();
    return publication;
  }

  for (const PlatformAccessibilityLiveUpdate& update : updates) {
    IRawElementProviderSimple* provider =
        find_provider(current_nodes, providers, update.element_id);
    if (provider == nullptr) {
      publication.missing_provider_count += 1;
      continue;
    }
    const Win32UiaProviderNode* previous =
        find_node(previous_nodes, update.element_id);
    if (update.kind == PlatformAccessibilityLiveUpdateKind::value_changed) {
      VARIANT old_value = bstr_variant(previous == nullptr ? "" : previous->value);
      VARIANT new_value = bstr_variant(update.value);
      if (old_value.bstrVal == nullptr || new_value.bstrVal == nullptr) {
        publication.last_status = E_OUTOFMEMORY;
        publication.failure_count += 1;
      } else if (operations.raise_property_changed == nullptr) {
        publication.last_status = E_POINTER;
        publication.failure_count += 1;
      } else {
        record_status(
            publication,
            operations.raise_property_changed(
                provider, UIA_ValueValuePropertyId, old_value, new_value),
            publication.property_changed_count);
      }
      VariantClear(&old_value);
      VariantClear(&new_value);
    } else if (update.kind ==
               PlatformAccessibilityLiveUpdateKind::text_changed) {
      const HRESULT status = operations.raise_automation_event == nullptr
          ? E_POINTER
          : operations.raise_automation_event(
                provider, UIA_Text_TextChangedEventId);
      record_status(
          publication, status, publication.automation_event_count);
    } else if (update.kind ==
               PlatformAccessibilityLiveUpdateKind::focus_changed) {
      const VARIANT old_value = bool_variant(
          previous == nullptr ? !update.focused : previous->focused);
      const VARIANT new_value = bool_variant(update.focused);
      const HRESULT property_status =
          operations.raise_property_changed == nullptr
          ? E_POINTER
          : operations.raise_property_changed(
                provider, UIA_HasKeyboardFocusPropertyId,
                old_value, new_value);
      record_status(
          publication, property_status,
          publication.property_changed_count);
      if (update.focused) {
        const HRESULT event_status = operations.raise_automation_event == nullptr
            ? E_POINTER
            : operations.raise_automation_event(
                  provider, UIA_AutomationFocusChangedEventId);
        record_status(
            publication, event_status,
            publication.automation_event_count);
      }
    }
  }
  return publication;
}

} // namespace cgpui
