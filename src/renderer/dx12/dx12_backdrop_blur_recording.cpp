#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"

namespace cgpui {
void dx12_begin_backdrop_blur_frame(Dx12State& state) {
  // Called only after successful idle wait. Never reuse logical resource
  // states from a command list that might not have reached the GPU.
  state.backdrop_blur->frame_targets.clear();
}

Result<void> dx12_record_backdrop_blur(Dx12State& state, const BackdropBlurPlan& plan) {
  for (const auto& pass : plan.passes) {
    if (!resolve_clip_stack_scissor(PixelExtent{state.width, state.height}, pass.clip_rect, {}).visible) continue;
    if (auto pipelines = dx12_create_backdrop_blur_pipelines(state); !pipelines) return pipelines;
    auto created = dx12_create_backdrop_blur_targets(state, pass.texels);
    if (!created) return std::unexpected(created.error());
    // Retain before recording any command referencing these resources.
    auto& targets = **created;
    state.backdrop_blur->frame_targets.push_back(std::move(*created));
    const UINT index = state.swapchain->GetCurrentBackBufferIndex();
    auto& backbuffer = *state.targets[index].Get();
    auto& snapshot = *targets.targets[0].resource.Get();
    dx12_transition(*state.commands.Get(), backbuffer,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
    state.commands->CopyResource(&snapshot, &backbuffer);
    dx12_transition(*state.commands.Get(), backbuffer,
        D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
    dx12_transition(*state.commands.Get(), snapshot,
        D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    dx12_record_backdrop_blur_passes(state, targets, pass);
    if (auto composite = dx12_record_backdrop_blur_composite(state, targets, pass, plan.vertices);
        !composite) return composite;
  }
  return {};
}
} // namespace cgpui
