#include "dx12_frame_recovery_internal.hpp"
#include "dx12_recovery_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
namespace {
Result<void> recovered_frame() {
  return std::unexpected(Error{.code = ErrorCode::frame_retry_required,
      .message = "DX12 device was rebuilt; request a new frame"});
}
} // namespace

Result<void> dx12_prepare_frame(Dx12State& state) {
  if (state.frame_active) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 already has an outstanding frame"});
  auto recovered = dx12_recover_removed_device(state);
  if (!recovered) return std::unexpected(recovered.error());
  if (*recovered) return recovered_frame();
  if (state.failed) return std::unexpected(dx12_unsupported("recovery of a failed live device"));
  return {};
}

Result<void> dx12_handle_frame_failure(Dx12State& state, Error error) {
  if (error.code == ErrorCode::frame_retry_required) return std::unexpected(std::move(error));
  state.failed = true;
  auto recovered = dx12_recover_removed_device(state);
  if (!recovered) return std::unexpected(recovered.error());
  if (*recovered) return recovered_frame();
  return std::unexpected(std::move(error));
}
} // namespace cgpui
