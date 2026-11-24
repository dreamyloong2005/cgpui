#include "dx12_foreign_probe_peer_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include <cstdlib>

namespace cgpui_test {
namespace {
int experiment(ForeignProbeMode mode, HANDLE cancel, ForeignProbeFixture& origin,
    ForeignProbePeer& peer, std::shared_ptr<cgpui::Dx12State>& pin) {
  ForeignProbeSnapshot before;
  if (!origin.initialize() || !origin.capture(true, false, cancel) || !origin.remember(before)) return 11;
  foreign_probe_log(foreign_probe_mode_name(mode),
      before.info.software_adapter ? "origin-pre-capture-software" : "origin-pre-capture-hardware",
      GetCurrentThreadId(), *before.info.adapter_luid, before.info.device_generation);
  if (!peer.start(mode, cancel)) return 12;
  if (foreign_probe_wait(peer.ready.get(), cancel, true, INFINITE) != ForeignProbeWait::signaled) return 13;
  pin = peer.take_state(); // Mutex synchronizes before/pacing; peer holds a stable local owner until release.
  if (!pin || !peer.pacing || peer.owner.load() == GetCurrentThreadId() ||
      before.info.adapter_luid != peer.before.info.adapter_luid) return 14;
  if (!foreign_probe_filter_excludes(*origin.state, pin)) return 15;
  if (!SetEvent(peer.go.get()) ||
      foreign_probe_wait(peer.entered.get(), cancel, true, INFINITE) != ForeignProbeWait::signaled) return 16;
  if (mode == ForeignProbeMode::pumping &&
      (!PostThreadMessageW(peer.owner.load(), foreign_probe_pulse, 0, 0) ||
       foreign_probe_wait(peer.pulse_seen.get(), cancel, true, INFINITE) != ForeignProbeWait::signaled)) return 17;
  foreign_probe_log(foreign_probe_mode_name(mode), "mode-established", peer.owner.load(),
      *before.info.adapter_luid, before.info.device_generation);
  // Real idle RemoveDevice, never TDR. Helper's device/InfoQueue COM refs die on return.
  if (!dx12_remove_native_device(before.info)) return 18;
  {
    cgpui::Dx12OperationGuard operation(*origin.state);
    if (!origin.state->device || SUCCEEDED(origin.state->device->GetDeviceRemovedReason()) ||
        origin.state->submitted) return 19;
  }
  // Deliberately NO origin gate held: isolate foreign native teardown itself.
  // The production origin-gate-held call graph is NOT certified by this probe.
  if (const int result = foreign_probe_retire(*pin, *peer.pacing, peer.before, mode)) return result;
  if (WaitForSingleObject(cancel, 0) != WAIT_TIMEOUT) return 20;
  if (!origin.recover_once(before) || !origin.capture(true, true, cancel) ||
      !dx12_recovered_native_device_clean(origin.state->info)) return 21;
  foreign_probe_log(foreign_probe_mode_name(mode), "origin-post-capture", GetCurrentThreadId(),
      *origin.state->info.adapter_luid, origin.state->info.device_generation);
  return 0;
}
} // namespace
int foreign_probe_case(ForeignProbeMode mode, HANDLE cancel) {
  DWORD flags = 0;
  if (!cancel || !GetHandleInformation(cancel, &flags) || _putenv_s("CGPUI_DX12_DEBUG", "1") != 0) return 10;
  int result = 29;
  {
    ForeignProbeFixture origin;
    ForeignProbePeer peer;
    std::shared_ptr<cgpui::Dx12State> pin;
    try { result = experiment(mode, cancel, origin, peer, pin); } catch (...) { result = 28; }
    pin.reset(); // Outside all gates, before allowing the owner to retire its final pin.
    const int peer_result = peer.finish(result == 0);
    if (!result && peer_result) result = peer_result;
    origin.state.reset(); // Peer completed; origin final pin also drops outside its operation gate.
    if (!result && (origin.pacing.closes != 2 || !origin.pacing.close_ok)) result = 22;
    // peer finishes before origin; both real HWNDs are destroyed on their creator.
  }
  if (!foreign_probe_output_ok() && !result) result = 27;
  foreign_probe_log(foreign_probe_mode_name(mode), result ? "CHILD_FAIL" : "CHILD_PASS", 0, 0, 0, result);
  return foreign_probe_output_ok() ? result : 27;
}
} // namespace cgpui_test
