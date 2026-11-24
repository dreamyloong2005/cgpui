#pragma once
#include "dx12_native_internal.hpp"
#include <array>

namespace cgpui {
struct Dx12State;
struct Dx12PreparedDraw {
  ID3D12PipelineState* pipeline = nullptr;
  ID3D12RootSignature* root = nullptr;
  std::array<float, 2> extent{};
  D3D12_VERTEX_BUFFER_VIEW vertex{};
  D3D12_INDEX_BUFFER_VIEW index{};
  ID3D12DescriptorHeap* srv_heap = nullptr;
  ID3D12DescriptorHeap* sampler_heap = nullptr;
  D3D12_GPU_DESCRIPTOR_HANDLE srv{}, sampler{};
  UINT count = 0, first = 0;
  bool indexed = false;
};
// Legal for both DIRECT and BUNDLE. Preparation, scissor and transitions are caller-owned.
void dx12_emit_prepared_draw(Dx12State&, ID3D12GraphicsCommandList&, const Dx12PreparedDraw&) noexcept;
} // namespace cgpui
