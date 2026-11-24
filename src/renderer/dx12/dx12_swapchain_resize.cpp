#include "dx12_state_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_command_reuse_surface_epoch_internal.hpp"
#include "dx12_swapchain_flags_internal.hpp"

namespace cgpui {
Result<void> dx12_resize(Dx12State& state, Size size, DpiScale scale) {
  if (auto valid = dx12_validate_extent(size, scale); !valid) return valid;
  if (state.frame_active) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 cannot resize while a frame is outstanding"});
  if (state.failed) return std::unexpected(dx12_unsupported("recovery of a failed device"));
  if (auto idle = dx12_wait_idle(state); !idle) { state.failed = true; return idle; }
  if (state.command_observation) state.command_observation->retire(false);
  const auto width = static_cast<UINT>(size.width), height = static_cast<UINT>(size.height);
  state.command_reuse->accept_surface({width, height, scale.value, state.composition_surface, state.transparent_background});
  state.scale = scale;
  if (width == state.width && height == state.height) return {};
  state.pixels.reset();
  state.readback.Reset();
  for (auto& target : state.targets) target.Reset();
  const HRESULT result = state.swapchain->ResizeBuffers(
      static_cast<UINT>(state.targets.size()), width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
      dx12_swapchain_flags());
  if (FAILED(result)) { state.failed = true; return std::unexpected(dx12_error(result, "resize buffers")); }
  state.width = width;
  state.height = height;
  auto targets = dx12_create_targets(state);
  if (!targets) state.failed = true;
  return targets;
}
} // namespace cgpui
