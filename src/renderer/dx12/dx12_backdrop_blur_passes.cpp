#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
void dx12_bind_backdrop_blur(Dx12State& state, const Dx12BackdropBlurTargets& targets,
    ID3D12PipelineState& pipeline, std::size_t source, const Dx12BackdropBlurConstants& constants) {
  state.commands->SetPipelineState(&pipeline);
  state.commands->SetGraphicsRootSignature(state.backdrop_blur->root_signature.Get());
  state.commands->SetGraphicsRoot32BitConstants(0, sizeof(constants) / sizeof(UINT), &constants, 0);
  ID3D12DescriptorHeap* heaps[] = {targets.srvs.Get()};
  state.commands->SetDescriptorHeaps(1, heaps);
  state.commands->SetGraphicsRootDescriptorTable(1, targets.targets[source].srv);
}

namespace {
void record_pass(Dx12State& state, const Dx12BackdropBlurTargets& targets,
    ID3D12PipelineState& pipeline, std::size_t source, std::size_t destination,
    PixelExtent texels, const Dx12BackdropBlurConstants& constants) {
  const auto& target = targets.targets[destination];
  state.commands->OMSetRenderTargets(1, &target.rtv, FALSE, nullptr);
  const D3D12_VIEWPORT viewport{0, 0, float(texels.width), float(texels.height), 0, 1};
  const D3D12_RECT scissor{0, 0, LONG(texels.width), LONG(texels.height)};
  state.commands->RSSetViewports(1, &viewport);
  state.commands->RSSetScissorRects(1, &scissor);
  dx12_bind_backdrop_blur(state, targets, pipeline, source, constants);
  state.commands->DrawInstanced(3, 1, 0, 0);
  dx12_transition(*state.commands.Get(), *target.resource.Get(),
      D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
} // namespace

void dx12_record_backdrop_blur_passes(Dx12State& state,
    const Dx12BackdropBlurTargets& targets, const BackdropBlurPass& pass) {
  const auto& renderer = *state.backdrop_blur;
  const Dx12BackdropBlurConstants downsample{
      .source_origin = {pass.region.x, pass.region.y},
      .source_max = {pass.region.x + std::int32_t(pass.region.width) - 1,
                     pass.region.y + std::int32_t(pass.region.height) - 1},
      .factor = std::int32_t(pass.factor)};
  record_pass(state, targets, *renderer.downsample_pipeline.Get(), 0, 1, pass.texels, downsample);
  Dx12BackdropBlurConstants blur{
      .source_max = {std::int32_t(pass.texels.width) - 1, std::int32_t(pass.texels.height) - 1},
      .direction = {1, 0}, .radius = pass.radius, .sigma = pass.sigma};
  record_pass(state, targets, *renderer.blur_pipeline.Get(), 1, 2, pass.texels, blur);
  dx12_transition(*state.commands.Get(), *targets.targets[1].resource.Get(),
      D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
  blur.direction = {0, 1};
  record_pass(state, targets, *renderer.blur_pipeline.Get(), 2, 1, pass.texels, blur);
}
} // namespace cgpui
