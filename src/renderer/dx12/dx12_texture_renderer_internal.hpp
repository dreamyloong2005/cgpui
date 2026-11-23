#pragma once

#include "dx12_texture_internal.hpp"

namespace cgpui {
struct Dx12TextureRenderer {
  Dx12Ptr<ID3D12RootSignature> root_signature;
  Dx12Ptr<ID3D12PipelineState> image_pipeline;
  Dx12Ptr<ID3D12PipelineState> text_pipeline;
  Dx12Ptr<ID3D12DescriptorHeap> samplers;
  UINT sampler_stride = 0;
  // Keep descriptors/resources alive even if a submitted frame's fence fails.
  // Release only after the next successful idle wait, never on frame teardown.
  std::vector<Dx12TextureHandle> retained;
  std::vector<Dx12TextureHandle> pending_uploads;
};

Result<void> dx12_create_texture_pipeline(Dx12State& state);
Result<void> dx12_create_textured_psos(Dx12State& state);
void dx12_begin_texture_frame(Dx12State& state);
void dx12_commit_texture_uploads(Dx12State& state);
} // namespace cgpui
