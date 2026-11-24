#include "dx12_state_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_synchronization_internal.hpp"
#include <limits>

namespace cgpui {
namespace {
Dx12SynchronizationProbe removed(Dx12State& state, HRESULT reason) noexcept {
  state.removal_reason = FAILED(reason) ? reason : DXGI_ERROR_DEVICE_REMOVED;
  state.command_reuse->removed(state);
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::removed, .result = state.removal_reason});
  state.pending_fence_value = 0;
  state.submitted = false;
  return {.progress = Dx12SynchronizationProgress::removed,
          .error = state.removal_reason, .operation = "device removed"};
}

Result<void> resolved(const Dx12SynchronizationProbe& probe) {
  if (probe.progress == Dx12SynchronizationProgress::complete) return {};
  const HRESULT error = FAILED(probe.error) ? probe.error : HRESULT_FROM_WIN32(ERROR_IO_INCOMPLETE);
  return std::unexpected(dx12_error(error, probe.operation));
}
} // namespace

Dx12SynchronizationProbe dx12_poll_submission(
    Dx12State& state, Dx12SynchronizationApi& api) noexcept {
  if (FAILED(state.removal_reason)) return removed(state, state.removal_reason);
  if (!state.submitted) return {.progress = Dx12SynchronizationProgress::complete};
  const HRESULT reason = api.removed(*state.device.Get());
  if (FAILED(reason)) return removed(state, reason);
  if (!state.pending_fence_value) {
    // UINT64_MAX is the device-removal sentinel, never a valid queue ticket.
    if (state.fence_value >= std::numeric_limits<UINT64>::max() - 1) {
      return {.error = HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW),
              .operation = "fence ticket exhaustion"};
    }
    const auto ticket = state.fence_value + 1;
    const HRESULT signaled = api.signal(*state.queue.Get(), *state.fence.Get(), ticket);
    if (FAILED(signaled)) return {.error = signaled, .operation = "queue signal"};
    state.fence_value = ticket;
    state.pending_fence_value = ticket;
  }
  const auto completed = api.completed(*state.fence.Get());
  if (completed == std::numeric_limits<UINT64>::max()) {
    return removed(state, api.removed(*state.device.Get()));
  }
  if (completed < state.pending_fence_value) return {};
  state.command_reuse->completed(state); // Actual same-ticket proof, not event wake or !submitted.
  if (state.command_observation) state.command_observation->completed(state, state.pending_fence_value);
  state.pending_fence_value = 0;
  state.submitted = false;
  return {.progress = Dx12SynchronizationProgress::complete};
}

Result<void> dx12_wait_idle(Dx12State& state) {
  Dx12NativeSynchronizationApi native;
  auto& api = state.synchronization_api ? *state.synchronization_api : native;
  const auto probe = dx12_poll_submission(state, api);
  if (probe.progress != Dx12SynchronizationProgress::pending || FAILED(probe.error)) {
    return resolved(probe);
  }
  const HRESULT registered = api.event(
      *state.fence.Get(), state.pending_fence_value, state.fence_event.value);
  if (FAILED(registered)) return std::unexpected(dx12_error(registered, "fence event registration"));
  const auto waited = api.wait(state.fence_event.value);
  if (waited.status != WAIT_OBJECT_0) {
    const DWORD error = waited.error ? waited.error : ERROR_GEN_FAILURE;
    return std::unexpected(dx12_error(HRESULT_FROM_WIN32(error), "fence wait"));
  }
  // A wake is only a notification. The same ticket must actually be complete.
  return resolved(dx12_poll_submission(state, api));
}

} // namespace cgpui
