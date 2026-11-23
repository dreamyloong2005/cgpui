#include "dx12_native_retirement_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_texture_renderer_internal.hpp"
#include "dx12_backdrop_blur_internal.hpp"

namespace cgpui {
void dx12_release_native_objects(Dx12State& state) noexcept {
  // The caller proved completion/removal, or never submitted the partial build.
  state.submitted = false;
  state.commands.Reset();
  state.allocator.Reset();
  dx12_reset_texture_resources(state);
  auto& backdrop = *state.backdrop_blur;
  backdrop.frame_targets.clear();
  backdrop.composite_pipeline.Reset();
  backdrop.blur_pipeline.Reset();
  backdrop.downsample_pipeline.Reset();
  backdrop.root_signature.Reset();
  state.uploads.clear();
  state.readback.Reset();
  state.capture_layout = {};
  state.shadow_pipeline.Reset();
  state.pipeline.Reset();
  state.root_signature.Reset();
  for (auto& target : state.targets) target.Reset();
  state.rtv_heap.Reset();
  state.rtv_stride = 0;
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
