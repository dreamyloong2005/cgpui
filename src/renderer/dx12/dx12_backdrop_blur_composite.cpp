#include "dx12_backdrop_blur_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_buffers_internal.hpp"
#include "cgpui/renderer/planning/clip_scissor.hpp"
#include "cgpui/renderer/planning/backdrop_blur_parameters.hpp"

namespace cgpui {
Result<void> dx12_record_backdrop_blur_composite(Dx12State& state,
    const Dx12BackdropBlurTargets& targets, const BackdropBlurPass& pass,
    std::span<const BackdropBlurVertex> vertices) {
  auto upload = dx12_upload_buffer(*state.device.Get(), vertices.data(), vertices.size_bytes());
  if (!upload) return std::unexpected(upload.error());
  const D3D12_VERTEX_BUFFER_VIEW vertex_view{(*upload)->GetGPUVirtualAddress(),
      UINT(vertices.size_bytes()), sizeof(BackdropBlurVertex)};
  const UINT index = state.swapchain->GetCurrentBackBufferIndex();
  auto rtv = state.rtv_heap->GetCPUDescriptorHandleForHeapStart();
  rtv.ptr += std::size_t(index) * state.rtv_stride;
  state.commands->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
  const D3D12_VIEWPORT viewport{0, 0, float(state.width), float(state.height), 0, 1};
  state.commands->RSSetViewports(1, &viewport);
  const auto clipped = resolve_clip_stack_scissor(
      PixelExtent{state.width, state.height}, pass.clip_rect, {});
  const auto clip = clipped.scissor;
  const D3D12_RECT scissor{clip.x, clip.y, LONG(clip.x + clip.width), LONG(clip.y + clip.height)};
  state.commands->RSSetScissorRects(1, &scissor);
  const Dx12BackdropBlurConstants constants{
      .framebuffer_size = {float(state.width), float(state.height)},
      .source_max = {std::int32_t(pass.texels.width) - 1, std::int32_t(pass.texels.height) - 1}};
  auto& renderer = *state.backdrop_blur;
  auto* pipeline = state.composition_surface ? renderer.alpha_composite_pipeline.Get()
                                            : renderer.composite_pipeline.Get();
  dx12_bind_backdrop_blur(state, targets, *pipeline, 1, constants);
  if (state.composition_surface)
    state.commands->SetGraphicsRootDescriptorTable(2, targets.targets[0].srv);
  state.commands->IASetVertexBuffers(0, 1, &vertex_view);
  state.commands->DrawInstanced(kBackdropBlurVerticesPerPass, 1, pass.first_vertex, 0);
  state.uploads.push_back(std::move(*upload));
  return {};
}
} // namespace cgpui
