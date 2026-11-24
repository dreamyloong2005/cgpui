#include "dx12_recovery_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_device_internal.hpp"
#include "dx12_device_cohort_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_recovery_work_internal.hpp"
#include <limits>

namespace cgpui {
namespace {
Result<void> rebuild(Dx12State& state, std::uint64_t luid) {
  if (auto device = dx12_create_device_on_adapter(state, luid); !device) return device;
  state.backdrop_blur->support = dx12_backdrop_blur_support(*state.device.Get());
  if (auto pipeline = dx12_create_pipeline(state); !pipeline) return pipeline;
  if (auto surface = dx12_create_swapchain(state); !surface) return surface;
  return dx12_check_debug_errors(state);
}
Result<void> await_retirement(Dx12State& state) {
  auto status = dx12_recovery_tickets().poll(state.recovery_ticket);
  if (status == Dx12RecoveryTicketStatus::pending) {
    dx12_dispatch_recovery_work(state);
    status = dx12_recovery_tickets().poll(state.recovery_ticket);
  }
  switch (status) {
    case Dx12RecoveryTicketStatus::ready: return {};
    case Dx12RecoveryTicketStatus::pending:
      return std::unexpected(Error{.code = ErrorCode::renderer_recovery_pending,
          .message = "DX12 recovery is waiting for owner-dispatched native retirement"});
    case Dx12RecoveryTicketStatus::timed_out:
      return std::unexpected(dx12_error(HRESULT_FROM_WIN32(ERROR_TIMEOUT), "cohort retirement deadline"));
    case Dx12RecoveryTicketStatus::endpoint_unavailable:
      return std::unexpected(dx12_error(HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED),
          "cohort retirement requires a managed owner-thread endpoint"));
    case Dx12RecoveryTicketStatus::post_failed:
      return std::unexpected(dx12_error(HRESULT_FROM_WIN32(ERROR_INVALID_WINDOW_HANDLE),
          "cohort retirement owner wake was not accepted"));
  }
  return std::unexpected(dx12_error(E_UNEXPECTED, "cohort retirement status"));
}
} // namespace

Result<bool> dx12_recover_removed_device(Dx12State& state) {
  const HRESULT actual = state.device ? state.device->GetDeviceRemovedReason() : S_OK;
  HRESULT reason = state.removal_reason;
  if (SUCCEEDED(reason)) reason = actual;
  if (!state.recovery_pending && SUCCEEDED(reason)) return false;
  if (!state.owner_alive) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 device reconstruction requires a live renderer owner"});
  // A nested callback cannot discard resources still used by its outer entry.
  if (state.operation_depth > 1) {
    return std::unexpected(dx12_error(HRESULT_FROM_WIN32(ERROR_BUSY), "reentrant device recovery"));
  }
  if (!state.recovery_pending) {
    state.removal_reason = reason;
    state.recovery_pending = true;
    state.failed = true;
  }
  state.pixels.reset();
  if (!state.info.adapter_luid) return std::unexpected(Error{
      .code = ErrorCode::renderer_initialization_failed,
      .message = "DX12 recovery requires the original adapter LUID"});
  if (state.info.device_generation == std::numeric_limits<std::uint64_t>::max()) {
    return std::unexpected(dx12_error(HRESULT_FROM_WIN32(ERROR_ARITHMETIC_OVERFLOW),
                                      "device generation exhaustion"));
  }
  const auto luid = *state.info.adapter_luid;
  const auto generation = state.info.device_generation;
  state.recovery_ticket = dx12_recovery_tickets().request(state.recovery_lease);
  if (FAILED(actual)) dx12_retire_removed_cohort_peers(state);
  dx12_release_native_objects(state);
  // No waiting, foreign gate acquisition or foreign native destruction here.
  // A pending result releases the origin's outer operation before owner pumps run.
  if (auto retired = await_retirement(state); !retired) return std::unexpected(retired.error());
  if (auto restored = rebuild(state, luid); !restored) {
    // Even a successful new device may have only partially initialized children.
    // It has submitted nothing, so this cleanup needs no completion wait.
    dx12_release_native_objects(state);
    return std::unexpected(restored.error());
  }
  state.fence_value = 0;
  state.pending_fence_value = 0;
  state.removal_reason = S_OK;
  state.failed = false;
  state.recovery_pending = false;
  state.recovery_ticket.reset();
  state.info.device_generation = generation + 1;
  return true;
}
} // namespace cgpui
