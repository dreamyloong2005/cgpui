#include "dx12_frame_recovery_internal.hpp"
#include "dx12_recovery_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
Result<void> dx12_resize_recovering(Dx12State& state, Size size, DpiScale scale) {
  // Invalid requests must not consume a device-recovery attempt or alter state.
  if (auto valid = dx12_validate_extent(size, scale); !valid) return valid;
  if (state.frame_active) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 cannot resize while a frame is outstanding"});
  auto recovered = dx12_recover_removed_device(state);
  if (!recovered) return std::unexpected(recovered.error());
  auto resized = dx12_resize(state, size, scale);
  if (resized || *recovered) return resized;
  // Removal may have occurred inside ResizeBuffers. Reconstruct at most once
  // in this call, then finish the requested resize; resize has no retry contract.
  recovered = dx12_recover_removed_device(state);
  if (!recovered) return std::unexpected(recovered.error());
  if (!*recovered) return resized;
  return dx12_resize(state, size, scale);
}
} // namespace cgpui
