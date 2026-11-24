#include "dx12_foreign_probe_peer_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"

namespace cgpui_test {
ForeignProbePeer::ForeignProbePeer()
    : ready(CreateEventW(nullptr, TRUE, FALSE, nullptr)), go(CreateEventW(nullptr, TRUE, FALSE, nullptr)),
      entered(CreateEventW(nullptr, TRUE, FALSE, nullptr)), release(CreateEventW(nullptr, TRUE, FALSE, nullptr)),
      pulse_seen(CreateEventW(nullptr, TRUE, FALSE, nullptr)), done(CreateEventW(nullptr, TRUE, FALSE, nullptr)) {}
ForeignProbePeer::~ForeignProbePeer() { (void)finish(false); }
bool ForeignProbePeer::start(ForeignProbeMode mode, HANDLE cancel) {
  if (!ready.get() || !go.get() || !entered.get() || !release.get() || !pulse_seen.get() || !done.get()) return false;
  worker_ = std::thread([this, mode, cancel] {
    int code = 79;
    try { code = run(mode, cancel); } catch (...) { code = 79; }
    // run's frame/state/COM/HWND locals have all retired before done.
    result.store(code);
    SetEvent(ready.get());
    SetEvent(done.get());
  });
  return true;
}
std::shared_ptr<cgpui::Dx12State> ForeignProbePeer::take_state() {
  std::weak_ptr<cgpui::Dx12State> weak;
  {
    std::lock_guard publication(publication_);
    weak = published_;
  }
  return weak.lock(); // Promotion and final pin release are outside metadata/state gates.
}
int ForeignProbePeer::run(ForeignProbeMode mode, HANDLE cancel) {
  ForeignProbeFixture fixture;
  if (!fixture.initialize() || !fixture.capture(false, false, cancel) || !fixture.remember(before)) return 71;
  owner.store(GetCurrentThreadId());
  {
    std::lock_guard publication(publication_);
    published_ = fixture.state;
    pacing = &fixture.pacing;
  }
  foreign_probe_log(foreign_probe_mode_name(mode), "peer-pre-capture", owner.load(),
      *before.info.adapter_luid, before.info.device_generation);
  SetEvent(ready.get());
  if (foreign_probe_wait(go.get(), cancel, true, INFINITE) != ForeignProbeWait::signaled) {
    (void)WaitForSingleObject(release.get(), INFINITE);
    return 72;
  }
  // After this event, parked mode executes no Peek/Dispatch or native operation
  // until release/cancel. Pumping mode's separate posted pulse proves servicing.
  SetEvent(entered.get());
  const auto waited = foreign_probe_wait(release.get(), cancel,
      mode == ForeignProbeMode::pumping, INFINITE, pulse_seen.get());
  if (waited != ForeignProbeWait::signaled) {
    // Cancellation never destroys HWND/pacing while the other thread still has
    // a pin or is inside native purge. Only release grants final owner teardown.
    (void)WaitForSingleObject(release.get(), INFINITE);
    return 73;
  }
  if (!recover_.load()) return 74;
  {
    cgpui::Dx12OperationGuard operation(*fixture.state);
    if (!foreign_probe_preserved(*fixture.state, before) || fixture.state->device ||
        fixture.pacing.closes != before.closes + 1) return 75;
  }
  if (!fixture.recover_once(before) || !fixture.capture(false, true, cancel) ||
      !dx12_recovered_native_device_clean(fixture.state->info)) return 76;
  foreign_probe_log(foreign_probe_mode_name(mode), "peer-post-capture", owner.load(),
      *fixture.state->info.adapter_luid, fixture.state->info.device_generation);
  fixture.state.reset(); // Coordinator already dropped its pin; no caller gate held.
  return fixture.pacing.closes == 2 && fixture.pacing.close_ok ? 0 : 77;
}
int ForeignProbePeer::finish(bool recover) {
  if (!worker_.joinable()) return result.load();
  recover_.store(recover);
  SetEvent(go.get());
  SetEvent(release.get());
  // No gate/registry lock is held; keep the origin HWND owner pumping through
  // peer native destruction. The independent parent bounds even stuck teardown.
  (void)foreign_probe_wait(done.get(), nullptr, true, INFINITE);
  worker_.join();
  return result.load();
}
} // namespace cgpui_test
