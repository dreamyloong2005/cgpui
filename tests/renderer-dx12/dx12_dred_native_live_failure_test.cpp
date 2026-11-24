#include "dx12_dred_native_test_support.hpp"
#include "dx12_sync_test_support.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"

namespace {
struct FalseSyncRemoval final : cgpui_test::Dx12SyncScript {
  unsigned removal_calls = 0;
  HRESULT removed(ID3D12Device&) noexcept override {
    ++removal_calls;
    return E_ACCESSDENIED; // Scripted sync failure only, NOT native device loss.
  }
};
}
int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;
  DredNativeReadout readout;
  FalseSyncRemoval script; // Outlives state; no actual command submission occurs.
  if (!deadline.done || !dred_select_debug(true)) return dx12_fail(1, "live false-loss setup");
  Dx12State state;
  Dx12OperationGuard gate(state);
  if (!dx12_create_device(state) || !dred_attach_readout(state, readout) ||
      state.device->GetDeviceRemovedReason() != S_OK)
    return dx12_fail(2, "actual native device is not live before scripted failure");
  if (!dred_record_identity(state.info, "live-before-script")) return dx12_fail(5, "live native identity/output failed");
  state.submitted = true; // Synthetic ownership path; zero ExecuteCommandLists.
  const auto probe = dx12_poll_submission(state, script);
  if (probe.progress != Dx12SynchronizationProgress::removed || probe.error != E_ACCESSDENIED ||
      script.removal_calls != 1 || script.signal_calls || state.removal_reason != E_ACCESSDENIED ||
      state.submitted || state.device->GetDeviceRemovedReason() != S_OK)
    return dx12_fail(3, "scripted sync failure vs live native reason distinction was not established");
  dx12_release_native_objects(state); // Real production hook must query actual device, not cached error.
  if (readout.calls != 1 || !readout.device_owned || readout.actual_reason != S_OK ||
      readout.snapshot.last_loss || state.dred_status.snapshot_locked().last_loss)
    return dx12_fail(4, "live native cleanup falsely published cached/injected failure as real removal");
  return dred_flush_output() ? 0 : dx12_fail(6, "live evidence output flush failed");
}
