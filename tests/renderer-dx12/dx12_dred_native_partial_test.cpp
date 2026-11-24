#include "dx12_dred_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  DredNativeReadout readout;
  if (!deadline.done || !dred_select_debug(true)) return dx12_fail(1, "DRED partial setup");
  Dx12State state;
  Dx12OperationGuard gate(state);
  if (!dx12_create_device(state) || !state.info.adapter_luid || !dred_attach_readout(state, readout))
    return dx12_fail(2, "DRED partial initial native device");
  if (!dred_record_identity(state.info, "partial-initial")) return dx12_fail(11, "partial native identity/output failed");
  const auto generation = state.info.device_generation;
  const auto luid_value = *state.info.adapter_luid;
  if (state.submitted || FAILED(state.device->GetDeviceRemovedReason()))
    return dx12_fail(3, "DRED partial fixture must start live and never submitted");
  dx12_release_native_objects(state); // Healthy cleanup supplies no scalar loss.
  if (readout.calls != 1 || readout.snapshot.last_loss || state.dred_status.snapshot_locked().last_loss)
    return dx12_fail(4, "healthy initial cleanup fabricated native removal");
  const LUID luid{DWORD(luid_value), LONG(luid_value >> 32)};
  if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&state.factory))) ||
      FAILED(state.factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&state.adapter))) ||
      FAILED(dx12_open_adapter_device(state)))
    return dx12_fail(5, "real device-only partial lease creation failed");
  if (state.queue || state.commands || state.allocator || state.fence || state.swapchain ||
      state.info.device_generation != generation || state.dred_status.snapshot_locked().last_loss)
    return dx12_fail(6, "partial lease unexpectedly published generation/children/history");
  if (!dred_record_identity(state.info, "partial-device-only")) return dx12_fail(11, "partial device identity/output failed");
  const auto first_loss = dred_remove_actual(*state.device.Get(), true);
  if (!first_loss.valid || FAILED(dx12_open_adapter_device(state)))
    return dx12_fail(7, "actual removed device-only direct reopen failed");
  const auto historical = state.dred_status.snapshot_locked();
  if (!historical.last_loss || !dred_status_matches(*historical.last_loss, first_loss) ||
      historical.last_is_current || historical.phase != Dx12DredSitePhase::open ||
      state.info.device_generation != generation || FAILED(state.device->GetDeviceRemovedReason()))
    return dx12_fail(8, "adapter direct-reset hook failed actual partial-loss scalar publication");
  if (!dred_record_identity(state.info, "partial-direct-reopened")) return dx12_fail(11, "partial reopened identity/output failed");
  const auto second_loss = dred_remove_actual(*state.device.Get(), true);
  if (!second_loss.valid) return dx12_fail(9, "second partial lease actual native removal failed");
  dx12_release_native_objects(state);
  if (readout.calls != 2 || !dred_current_readout(readout, second_loss, generation) ||
      state.device || state.info.device_generation != generation ||
      state.dred_status.snapshot_locked().last_is_current)
    return dx12_fail(10, "new partial lease at same public generation missed current common publication");
  return dred_flush_output() ? 0 : dx12_fail(12, "partial evidence output flush failed");
}
