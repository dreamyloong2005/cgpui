#include "dx12_recovery_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_device_internal.hpp"
#include "dx12_native_retirement_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
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
} // namespace

Result<bool> dx12_recover_removed_device(Dx12State& state) {
  if (!state.recovery_pending) {
    HRESULT reason = state.removal_reason;
    if (SUCCEEDED(reason) && state.device) reason = state.device->GetDeviceRemovedReason();
    if (SUCCEEDED(reason)) return false;
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
  dx12_release_native_objects(state);
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
  state.info.device_generation = generation + 1;
  return true;
}
} // namespace cgpui
