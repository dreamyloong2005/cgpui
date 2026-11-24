#include "dx12_orphan_frame_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include <utility>

namespace cgpui {
namespace {
Error unavailable(const char* message) {
  return {.code = ErrorCode::invalid_argument, .message = message};
}
void retire_removed_epoch(Dx12State& state) {
  const HRESULT actual = state.device ? state.device->GetDeviceRemovedReason() : S_OK;
  const HRESULT reason = FAILED(actual) ? actual : state.removal_reason;
  if (SUCCEEDED(reason)) return; // Live incomplete work must remain owned.
  state.removal_reason = reason;
  state.failed = true;
  state.recovery_pending = true;
  // Same proof/depth rule as native recovery, but never reconstruction or peer rebuilding.
  if (state.device && state.operation_depth <= 1) dx12_release_native_objects(state);
}
}

Result<void> dx12_prepare_orphan_present(Dx12State& state, std::uint64_t generation) {
  retire_removed_epoch(state);
  if (!state.device || state.failed || state.recovery_pending ||
      generation != state.info.device_generation)
    return std::unexpected(unavailable("DX12 orphan frame no longer owns its healthy native epoch"));
  if (!state.hwnd || !IsWindow(state.hwnd))
    return std::unexpected(unavailable("DX12 orphan frame requires its live native window"));
  if (auto valid = dx12_validate_window_alpha(state); !valid) return valid;
  if (!state.swapchain || !state.commands || !state.allocator || !state.queue || !state.fence)
    return std::unexpected(unavailable("DX12 orphan frame native presentation objects are unavailable"));
  return {};
}

Result<void> dx12_fail_orphan_frame(Dx12State& state, Error error) {
  state.failed = true;
  retire_removed_epoch(state);
  // There is no facade from which to obtain a new frame. Never advertise a retry.
  if (error.code == ErrorCode::frame_retry_required || error.code == ErrorCode::frame_not_ready)
    return std::unexpected(unavailable("DX12 orphan frame cannot retry or rebuild its native epoch"));
  return std::unexpected(std::move(error));
}
} // namespace cgpui
