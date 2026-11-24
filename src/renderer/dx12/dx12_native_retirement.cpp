#include "dx12_native_retirement_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_geometry_upload_internal.hpp"
#include "dx12_texture_upload_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include "dx12_srv_descriptor_pool_internal.hpp"

namespace cgpui {
void dx12_release_native_objects(Dx12State& state) noexcept {
  // The caller proved completion/removal, or never submitted the partial build.
  if (FAILED(state.removal_reason)) state.command_reuse->removed(state); // Covers actual removed cohort/orphan paths too.
  state.submitted = false;
  auto* old_parent = state.commands.Get();
  state.commands.Reset();
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::parent_released, .object = old_parent});
  state.allocator.Reset();
  state.command_reuse->parent_released(state); // Actual P; entry releases before texture map/glyph/SRV pool.
  if (state.command_observation) state.command_observation->retire(FAILED(state.removal_reason));
  dx12_reset_texture_resources(state);
  auto& backdrop = *state.backdrop_blur;
  backdrop.frame_targets.clear();
  state.srv_pool->clear_after_retirement();
  backdrop.composite_pipeline.Reset();
  backdrop.alpha_composite_pipeline.Reset();
  backdrop.blur_pipeline.Reset();
  backdrop.downsample_pipeline.Reset();
  backdrop.root_signature.Reset();
  state.geometry_uploads->clear();
  state.texture_uploads->clear();
  state.readback.Reset();
  state.capture_layout = {};
  state.shadow_pipeline.Reset();
  state.pipeline.Reset();
  state.root_signature.Reset();
  for (auto& target : state.targets) target.Reset();
  state.rtv_heap.Reset();
  state.rtv_stride = 0;
  dx12_release_composition(state);
  state.frame_pacing->reset();
  state.swapchain.Reset();
  state.queue.Reset();
  state.fence.Reset();
  if (state.fence_event.value) CloseHandle(state.fence_event.value);
  state.fence_event.value = nullptr;
  state.device.Reset();
  state.adapter.Reset();
  state.factory.Reset();
}
} // namespace cgpui
