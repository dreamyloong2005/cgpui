#include "dx12_owner_recovery_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"

namespace cgpui_test {
using namespace cgpui;
namespace {
std::shared_ptr<Dx12State> find_state(const ManagedRecoveryWindow& fixture) {
  Dx12State query;
  query.info = fixture.renderer->backend_info();
  query.window_thread_id = GetCurrentThreadId();
  const auto hwnd = std::get<Win32SurfaceHandle>(fixture.window->native_surface()).hwnd;
  for (const auto& weak : dx12_device_cohort_peers(query)) {
    auto state = weak.lock();
    if (state && state->hwnd == hwnd) return state;
  }
  return {};
}
void yield_owner() { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
} // namespace

OwnerRecoveryPeer::OwnerRecoveryPeer(OwnerRecoveryCase mode)
    : mode_(mode), worker_([this] { result = run(); if (!prepared) ready.count_down(); }) {}
OwnerRecoveryPeer::~OwnerRecoveryPeer() { (void)finish(); }
int OwnerRecoveryPeer::finish() {
  stop.store(true);
  if (worker_.joinable()) worker_.join();
  return result;
}
int OwnerRecoveryPeer::run() {
  ManagedRecoveryWindow fixture;
  if (!fixture.renderer) return 1;
  auto state = find_state(fixture);
  if (!state || !state->recovery_work_token) return 2;
  if (!owner_recovery_paint(*fixture.renderer, true)) return 3;
  auto old_frame = wait_for_test_frame(*fixture.renderer, {.message_owner = std::this_thread::get_id()});
  if (!old_frame) return 4;
  info = fixture.renderer->backend_info();
  const auto* capture = fixture.renderer->last_frame_pixels();
  const auto saved = capture->rgba8;
  prepared = true;
  ready.count_down();
  if (mode_ == OwnerRecoveryCase::parked) {
    while (!stop.load()) yield_owner();
    if (fixture.renderer->last_frame_pixels() != capture || capture->rgba8 != saved ||
        fixture.renderer->backend_info().device_generation != info.device_generation || !state->device) return 5;
    return 0; // Destruction itself must release/ack, without ever pumping the request.
  }
  while (!pump.load() && !stop.load()) yield_owner();
  if (stop.load()) return 0;
  if (mode_ == OwnerRecoveryCase::busy) {
    Dx12OperationGuard outer(*state);
    auto* device = state->device.Get();
    bool dispatched = false;
    const auto busy_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(1);
    while (!dispatched && !stop.load() && std::chrono::steady_clock::now() < busy_deadline) {
      dispatched = owner_recovery_pump(); // Callback sees same-thread recursive depth, not idle.
      yield_owner();
    }
    if (!dispatched || state->device.Get() != device || state->recovery_pending ||
        capture->rgba8 != saved || state->info.device_generation != info.device_generation) return 6;
    busy_seen.store(true);
    while (!release_busy.load() && !stop.load()) yield_owner();
  } // The explicit outer-operation exit must requeue deferred retirement.
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (state->device && !stop.load() && std::chrono::steady_clock::now() < deadline) {
    owner_recovery_pump();
    yield_owner();
  }
  if (stop.load()) return 0;
  if (state->device || !state->recovery_pending || !state->frame_active ||
      capture->rgba8 != saved || state->info.device_generation != info.device_generation ||
      fixture.renderer->last_frame_pixels()) return 7;
  retired.store(true);
  while (!origin_rebuilt.load() && !stop.load()) yield_owner();
  if (stop.load()) return 0;
  const auto old_present = (*old_frame)->present();
  if (old_present || old_present.error().code != ErrorCode::frame_retry_required) return 8;
  const auto after = fixture.renderer->backend_info();
  if (after.device_generation != info.device_generation + 1 || after.adapter_luid != info.adapter_luid ||
      !owner_recovery_paint(*fixture.renderer, false) || !dx12_recovered_native_device_clean(after)) return 9;
  completed.store(true);
  while (!stop.load()) { owner_recovery_pump(); yield_owner(); }
  return 0;
}
} // namespace cgpui_test
