#pragma once
#include "dx12_draw_commands_internal.hpp"
#include <cstdint>
#include <span>

namespace cgpui {
struct Dx12ReadyImageBinding {
  ID3D12Resource* resource = nullptr;
  ID3D12PipelineState* pipeline = nullptr;
  ID3D12RootSignature* root = nullptr;
  ID3D12DescriptorHeap* heap = nullptr;
  ID3D12DescriptorHeap* samplers = nullptr;
  D3D12_GPU_DESCRIPTOR_HANDLE srv{}, sampler{};
};
struct Dx12ReuseScene {
  std::vector<std::uint64_t> signature;
  std::vector<Dx12TextureHandle> pins;
  std::vector<Dx12ReadyImageBinding> bindings; // One element per original group; empty for geometry.
  D3D12_RECT scissor{};
  ID3D12DescriptorHeap* heap = nullptr;
  ID3D12DescriptorHeap* samplers = nullptr;
};
bool dx12_reuse_signature_equal(const Dx12ReuseScene&, const Dx12ReuseScene&) noexcept;
void dx12_append_reuse_signature(Dx12ReuseScene&, std::span<const Dx12DrawCommand>);
Result<std::optional<Dx12ReuseScene>> dx12_select_reuse_scene(
    Dx12State&, Color, std::span<const Dx12DrawCommand>, bool capture, bool mutations, std::uint64_t surface);
} // namespace cgpui
