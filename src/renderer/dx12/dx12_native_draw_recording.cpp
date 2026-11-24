#include "dx12_prepared_draw_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include <bit>

namespace cgpui {
void dx12_emit_prepared_draw(Dx12State& state, ID3D12GraphicsCommandList& list,
                             const Dx12PreparedDraw& draw) noexcept {
  using K = Dx12CommandEventKind;
  list.SetPipelineState(draw.pipeline);
  dx12_observe_command(state, {.kind = K::pipeline, .list = &list, .object = draw.pipeline});
  list.SetGraphicsRootSignature(draw.root);
  dx12_observe_command(state, {.kind = K::root, .list = &list, .object = draw.root});
  list.SetGraphicsRoot32BitConstants(0, 2, draw.extent.data(), 0);
  dx12_observe_command(state, {.kind = K::constants, .list = &list,
      .a = std::bit_cast<std::uint32_t>(draw.extent[0]), .b = std::bit_cast<std::uint32_t>(draw.extent[1])});
  list.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  dx12_observe_command(state, {.kind = K::topology, .list = &list, .a = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST});
  if (draw.srv_heap) {
    ID3D12DescriptorHeap* heaps[] = {draw.srv_heap, draw.sampler_heap};
    list.SetDescriptorHeaps(2, heaps);
    dx12_observe_command(state, {.kind = K::heaps, .list = &list, .object = heaps[0], .object2 = heaps[1]});
    list.SetGraphicsRootDescriptorTable(1, draw.srv);
    dx12_observe_command(state, {.kind = K::table, .list = &list, .a = 1, .b = draw.srv.ptr});
    list.SetGraphicsRootDescriptorTable(2, draw.sampler);
    dx12_observe_command(state, {.kind = K::table, .list = &list, .a = 2, .b = draw.sampler.ptr});
  }
  list.IASetVertexBuffers(0, 1, &draw.vertex);
  dx12_observe_command(state, {.kind = K::vertex, .list = &list,
      .a = draw.vertex.BufferLocation, .b = draw.vertex.SizeInBytes, .c = draw.vertex.StrideInBytes});
  if (draw.indexed) {
    list.IASetIndexBuffer(&draw.index);
    dx12_observe_command(state, {.kind = K::index, .list = &list,
        .a = draw.index.BufferLocation, .b = draw.index.SizeInBytes,
        .c = static_cast<std::uint64_t>(draw.index.Format)});
    list.DrawIndexedInstanced(draw.count, 1, draw.first, 0, 0);
    dx12_observe_command(state, {.kind = K::indexed_draw, .list = &list, .a = draw.count, .b = draw.first});
  } else {
    list.DrawInstanced(draw.count, 1, draw.first, 0);
    dx12_observe_command(state, {.kind = K::draw, .list = &list, .a = draw.count, .b = draw.first});
  }
}
} // namespace cgpui
