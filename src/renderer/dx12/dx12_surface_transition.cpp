#include "dx12_surface_transition_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_swapchain_candidate_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_composition_api_internal.hpp"

namespace cgpui {
Result<void> dx12_replace_surface_alpha(Dx12State& state, bool premultiplied) {
  if (state.surface_changing || state.operation_depth > 1)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 nested alpha replacement is not permitted"});
  if (!state.surface_policy.dynamic_ordinary || !dx12_composition_route(state.surface_policy) ||
      !state.composition_host || !state.window_alpha)
    return std::unexpected(dx12_unsupported("ordinary alpha route replacement"));
  auto& host = *state.composition_host;
  if (state.window_alpha->thread != GetCurrentThreadId())
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 ordinary alpha changes require the HWND owner thread"});
  if (state.frame_active)
    return std::unexpected(Error{.code = ErrorCode::invalid_argument,
        .message = "DX12 alpha replacement requires no outstanding frame"});
  if (state.failed || state.recovery_pending)
    return std::unexpected(dx12_unsupported("alpha replacement during failed or pending device recovery"));
  struct Changing {
    bool& active;
    explicit Changing(bool& value) : active(value) { active = true; }
    ~Changing() { active = false; }
  } changing(state.surface_changing);
  if (auto idle = dx12_wait_idle(state); !idle) { state.failed = true; return idle; }
  // Prepare every native owner, including an independent pacing handle, before
  // changing window alpha or the still-visible old composition visual.
  auto built = dx12_prepare_composition_chain(state, premultiplied);
  if (!built) return std::unexpected(built.error());
  auto candidate = std::move(*built);
  if (auto clean = dx12_check_debug_errors(state); !clean) return clean;
  const bool promoted = premultiplied && !state.window_alpha->enabled;
  if (promoted) {
    if (auto alpha = dx12_acquire_window_alpha(state); !alpha) return alpha;
  }
  const HRESULT bound = dx12_bind_composition(host, candidate->swapchain.Get());
  if (FAILED(bound)) {
    // No successful replacement or intent publication on partial native failure.
    const HRESULT restored = dx12_bind_composition(host, state.swapchain.Get());
    if (FAILED(restored)) {
      state.failed = true;
      host.failed_binding = std::move(candidate); // Retain every possibly bound chain until terminal teardown.
    }
    if (promoted) {
      if (auto alpha = dx12_suspend_window_alpha(state); !alpha) return alpha;
    }
    return std::unexpected(dx12_error(FAILED(restored) ? restored : bound, "composition replacement binding"));
  }
  dx12_exchange_swapchain(state, *candidate); // candidate now owns the retired old chain.
  // Coalesce uncommitted toggles: retain the ORIGINAL visible chain, not an
  // unbounded chain history. A previous pending chain cannot have a successful
  // frame (that would Commit/clear retired_chain); native frame failures poison
  // state and were rejected above. It is unsubmitted/idle and will not execute.
  if (!host.retired_chain) host.retired_chain = std::move(candidate);
  host.committed = false;
  state.composition_surface = premultiplied;
  // Alpha-specific backdrop roots cannot be reused across opaque/PREMULT modes.
  // GPU completion is established above; next frame creates the correct variant.
  auto& blur = *state.backdrop_blur;
  blur.frame_targets.clear();
  blur.composite_pipeline.Reset(); blur.alpha_composite_pipeline.Reset();
  blur.blur_pipeline.Reset(); blur.downsample_pipeline.Reset(); blur.root_signature.Reset();
  state.readback.Reset();
  // Preserve last successful pixels/diagnostics until the next successful frame.
  // The existing presentation gate performs Commit before publishing either.
  return {};
}
} // namespace cgpui
