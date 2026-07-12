#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include <UIAutomationClient.h>
#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>

#include "win32_accessibility_internal.hpp"

#include <atomic>
#include <string>
#include <thread>

namespace {

cgpui::PlatformAccessibilityTreeUpdate make_update(
    std::size_t revision,
    bool include_input = true) {
  cgpui::PlatformAccessibilityTreeUpdate update{
      .root_element_id = 1,
      .node_count = include_input ? 2U : 1U,
      .nodes = {{.element_id = 1,
                 .role = cgpui::PlatformAccessibilityRole::generic,
                 .name = "Root",
                 .child_count = include_input ? 1U : 0U}}};
  if (include_input) {
    update.nodes.push_back({
        .element_id = 2,
        .parent_element_id = 1,
        .role = cgpui::PlatformAccessibilityRole::text_input,
        .name = "Input",
        .value = "revision-" + std::to_string(revision),
        .patterns = {.value_settable = true},
        .enabled = true,
        .focusable = true,
        .focused = revision % 2U == 0U,
        .bounds = cgpui::Rect{{10.0F, 20.0F}, {180.0F, 32.0F}},
    });
  }
  return update;
}

IRawElementProviderFragment* fragment_for(
    IRawElementProviderSimple* provider) {
  IRawElementProviderFragment* fragment = nullptr;
  return provider != nullptr && provider->QueryInterface(
      IID_IRawElementProviderFragment,
      reinterpret_cast<void**>(&fragment)) == S_OK
      ? fragment
      : nullptr;
}

bool value_is_revision(IRawElementProviderSimple* provider) {
  VARIANT value{};
  VariantInit(&value);
  const HRESULT status = provider->GetPropertyValue(
      UIA_ValueValuePropertyId, &value);
  const bool valid = status == S_OK && value.vt == VT_BSTR &&
      value.bstrVal != nullptr &&
      std::wstring{value.bstrVal, SysStringLen(value.bstrVal)}.starts_with(
          L"revision-");
  VariantClear(&value);
  return valid;
}

bool runtime_id_is_input(IRawElementProviderFragment* fragment) {
  SAFEARRAY* runtime_id = nullptr;
  if (fragment->GetRuntimeId(&runtime_id) != S_OK || runtime_id == nullptr) {
    return false;
  }
  LONG* values = nullptr;
  const bool valid = SafeArrayAccessData(
          runtime_id, reinterpret_cast<void**>(&values)) == S_OK &&
      values[0] == UiaAppendRuntimeId && values[1] == 0 && values[2] == 2;
  if (values != nullptr) SafeArrayUnaccessData(runtime_id);
  SafeArrayDestroy(runtime_id);
  return valid;
}

} // namespace

int main() {
  cgpui::Win32UiaAccessibilityAdapter adapter;
  adapter.update(make_update(0));
  IRawElementProviderSimple* input = adapter.provider_for_element(2);
  IRawElementProviderFragment* input_fragment = fragment_for(input);
  IRawElementProviderFragment* root_fragment = fragment_for(
      adapter.root_provider());
  if (input == nullptr || input_fragment == nullptr || root_fragment == nullptr) {
    return 1;
  }
  input->AddRef();
  auto diagnostics = adapter.last_lifetime_diagnostics();
  if (diagnostics.created_count != 2 || diagnostics.reused_count != 0 ||
      diagnostics.retired_count != 0 || diagnostics.active_count != 2) return 2;

  std::atomic<bool> stop{false};
  std::atomic<bool> failed{false};
  std::thread reader([&] {
    while (!stop.load(std::memory_order_acquire)) {
      IRawElementProviderFragment* parent = nullptr;
      if (!value_is_revision(input) || !runtime_id_is_input(input_fragment) ||
          input_fragment->Navigate(NavigateDirection_Parent, &parent) != S_OK ||
          parent != root_fragment) {
        failed.store(true, std::memory_order_release);
      }
      if (parent != nullptr) parent->Release();
    }
  });

  for (std::size_t revision = 1; revision <= 128; ++revision) {
    adapter.update(make_update(revision));
    diagnostics = adapter.last_lifetime_diagnostics();
    if (adapter.provider_for_element(2) != input ||
        diagnostics.created_count != 0 || diagnostics.reused_count != 2 ||
        diagnostics.retired_count != 0 || diagnostics.active_count != 2) {
      failed.store(true, std::memory_order_release);
    }
  }
  stop.store(true, std::memory_order_release);
  reader.join();
  if (failed.load(std::memory_order_acquire)) return 3;
  input_fragment->Release();
  root_fragment->Release();
  input->Release();

  for (std::size_t cycle = 0; cycle < 64; ++cycle) {
    IRawElementProviderSimple* retired = adapter.provider_for_element(2);
    retired->AddRef();
    adapter.update(make_update(cycle, false));
    VARIANT unavailable{};
    diagnostics = adapter.last_lifetime_diagnostics();
    if (retired->GetPropertyValue(UIA_NamePropertyId, &unavailable) !=
            UIA_E_ELEMENTNOTAVAILABLE ||
        diagnostics.created_count != 0 || diagnostics.reused_count != 1 ||
        diagnostics.retired_count != 1 || diagnostics.active_count != 1) {
      return 4;
    }
    adapter.update(make_update(cycle + 129));
    IRawElementProviderSimple* replacement = adapter.provider_for_element(2);
    diagnostics = adapter.last_lifetime_diagnostics();
    if (replacement == nullptr || replacement == retired ||
        diagnostics.created_count != 1 || diagnostics.reused_count != 1 ||
        diagnostics.retired_count != 0 || diagnostics.active_count != 2) {
      return 5;
    }
    retired->Release();
  }
  return 0;
}
