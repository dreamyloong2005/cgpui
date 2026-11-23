#pragma once

#include "dx12_texture_realization_internal.hpp"
#include <unordered_map>

namespace cgpui {
struct Dx12TextureRenderer {
  Dx12Ptr<ID3D12RootSignature> root_signature;
  Dx12Ptr<ID3D12PipelineState> image_pipeline;
  Dx12Ptr<ID3D12PipelineState> text_pipeline;
  Dx12Ptr<ID3D12DescriptorHeap> samplers;
  UINT sampler_stride = 0;
  std::unordered_map<const Dx12Texture*, Dx12NativeTexture> native_textures;
  // Retain CPU snapshots during submission. Native map nodes remain stable on
  // rehash and are pruned only after successful frame-start idle completion.
  std::vector<Dx12TextureHandle> retained;
  std::vector<Dx12NativeTexture*> pending_uploads;
};

Result<void> dx12_create_texture_pipeline(Dx12State& state);
Result<void> dx12_create_textured_psos(Dx12State& state);
Result<Dx12NativeTexture*> dx12_prepare_texture(
    Dx12State& state, const Dx12TextureHandle& texture);
void dx12_begin_texture_frame(Dx12State& state);
void dx12_commit_texture_uploads(Dx12State& state);
// Requires completed submission or confirmed removal; keeps CPU caches intact.
void dx12_reset_texture_resources(Dx12State& state) noexcept;
} // namespace cgpui
