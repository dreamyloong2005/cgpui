#include "dx12_dred_native_test_support.hpp"

namespace cgpui_test {
DredNativeLoss dred_remove_actual(ID3D12Device& source, bool debug) {
  DredNativeLoss observed{.debug = debug};
  Microsoft::WRL::ComPtr<ID3D12Device5> device;
  Microsoft::WRL::ComPtr<ID3D12InfoQueue> messages;
  if (FAILED(source.QueryInterface(IID_PPV_ARGS(&device))) || !device ||
      FAILED(device->GetDeviceRemovedReason())) return observed;
  (void)device.As(&messages);
  if ((debug && !messages) || !dx12_recovery_debug_messages_safe(messages.Get())) return observed;
  if (messages) messages->ClearStoredMessages();
  device->RemoveDevice(); // Actual idle native removal, never a manufactured HRESULT.
  observed.reason = device->GetDeviceRemovedReason();
  if (SUCCEEDED(observed.reason) || !dx12_recovery_debug_messages_safe(messages.Get(), true)) return observed;
  if (debug) {
    // Independent reference observation, not production's once-per-lease query counter.
    Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedData> dred;
    const HRESULT queried = device.As(&dred);
    observed.query = {static_cast<std::int32_t>(queried), dred.Get() != nullptr};
    dred_observe_breadcrumb_reference(source, observed);
  }
  observed.valid = true;
  std::cout << "DRED actual idle removal reason=" << std::hex << observed.reason << " debug=" << debug;
  if (debug) std::cout << " reference_qi=" << observed.query.hresult << " interface_received=" << observed.query.received;
  else std::cout << " reference_qi=not_attempted(debug_off)";
  std::cout << std::dec << '\n';
  return observed; // All temporary device/InfoQueue/DRED refs are released before caller recovery.
}
DredNativeLoss dred_remove_actual(const cgpui::RendererBackendInfo& info, bool debug) {
  const auto device = dx12_acquire_native_device(info);
  return device ? dred_remove_actual(*device.Get(), debug) : DredNativeLoss{};
}
bool dred_status_matches(const cgpui::Dx12DredStatus& status, const DredNativeLoss& observed) {
  using namespace cgpui;
  if (!observed.valid || status.actual_removed_reason != observed.reason ||
      status.scope != Dx12DredScope::site_local || status.native_epoch != Dx12DredUnknown::unknown ||
      status.breadcrumb_count != Dx12DredUnknown::unknown ||
      status.participant_coverage != Dx12DredUnknown::unknown ||
      status.native_device_setup_coverage != Dx12DredUnknown::unknown) return false;
  if (!observed.debug) return std::holds_alternative<Dx12DredNotRequested>(status.setup) &&
      std::holds_alternative<Dx12DredDebugNotRequested>(status.interface_query) &&
      status.output == Dx12DredOutput::not_requested_boundary_unproven &&
      !status.breadcrumbs.output_requested;
  if (std::holds_alternative<Dx12DredNotRequested>(status.setup)) return false;
  const auto* query = std::get_if<Dx12DredInterfaceResult>(&status.interface_query);
  if (!query || query->hresult != observed.query.hresult || query->received != observed.query.received) return false;
  const bool output_expected = query->hresult >= 0 && query->received;
  if (status.breadcrumbs.output_requested != observed.breadcrumb_requested ||
      (output_expected && status.breadcrumbs.output_hresult != observed.breadcrumb_hresult) ||
      status.breadcrumbs.nodes > 64 || status.breadcrumbs.operations > status.breadcrumbs.nodes * 32 ||
      status.breadcrumbs.output_requested != output_expected ||
      (output_expected && (status.output != Dx12DredOutput::best_effort_non_atomic ||
       status.breadcrumbs.status == Dx12DredBreadcrumbStatus::not_attempted))) return false;
  if (const auto* returned = std::get_if<Dx12DredSetupRequestReturned>(&status.setup))
    return returned->query.hresult >= 0 && returned->query.received;
  const auto* unavailable = std::get_if<Dx12DredSetupUnavailable>(&status.setup);
  return unavailable && (unavailable->query.hresult < 0 || !unavailable->query.received);
}
} // namespace cgpui_test
