#pragma once

#include "dx12_native_internal.hpp"
#include "cgpui/renderer/planning/backdrop_blur_plan.hpp"
#include <array>
#include <memory>
#include <vector>

namespace cgpui {
struct Dx12State;
struct Dx12BackdropBlurConstants {
  std::array<float, 2> framebuffer_size{};
  std::array<std::int32_t, 2> source_origin{};
  std::array<std::int32_t, 2> source_max{};
  std::array<std::int32_t, 2> direction{};
  std::int32_t factor = 1;
  std::int32_t radius = 0;
  float sigma = 0;
  float padding = 0;
};
static_assert(sizeof(Dx12BackdropBlurConstants) == 48);

struct Dx12BackdropBlurTarget {
  Dx12Ptr<ID3D12Resource> resource;
  D3D12_GPU_DESCRIPTOR_HANDLE srv{};
  D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
};
struct Dx12BackdropBlurTargets {
  // Snapshot, ping, pong. A new set per effect keeps already-recorded
  // resources and their descriptor contents immutable until GPU completion.
  std::array<Dx12BackdropBlurTarget, 3> targets;
  Dx12Ptr<ID3D12DescriptorHeap> srvs;
  Dx12Ptr<ID3D12DescriptorHeap> rtvs;
};
struct Dx12BackdropBlurRenderer {
  RendererEffectSupport support;
  Dx12Ptr<ID3D12RootSignature> root_signature;
  Dx12Ptr<ID3D12PipelineState> downsample_pipeline;
  Dx12Ptr<ID3D12PipelineState> blur_pipeline;
  Dx12Ptr<ID3D12PipelineState> composite_pipeline;
  Dx12Ptr<ID3D12PipelineState> alpha_composite_pipeline;
  std::vector<std::unique_ptr<Dx12BackdropBlurTargets>> frame_targets;
};

RendererEffectSupport dx12_backdrop_blur_support(ID3D12Device& device);
bool dx12_valid_backdrop_blur_input(const BackdropBlurDraw& draw);
bool dx12_valid_backdrop_blur_plan(const BackdropBlurPlan& plan, PixelExtent extent);
Result<void> dx12_create_backdrop_blur_root(Dx12State& state);
Result<void> dx12_create_backdrop_blur_pipelines(Dx12State& state);
Result<std::unique_ptr<Dx12BackdropBlurTargets>> dx12_create_backdrop_blur_targets(
    Dx12State& state, PixelExtent texels);
void dx12_begin_backdrop_blur_frame(Dx12State& state);
void dx12_bind_backdrop_blur(Dx12State& state, const Dx12BackdropBlurTargets& targets,
    ID3D12PipelineState& pipeline, std::size_t source, const Dx12BackdropBlurConstants& constants);
void dx12_record_backdrop_blur_passes(Dx12State& state,
    const Dx12BackdropBlurTargets& targets, const BackdropBlurPass& pass);
Result<void> dx12_record_backdrop_blur_composite(Dx12State& state,
    const Dx12BackdropBlurTargets& targets, const BackdropBlurPass& pass,
    std::span<const BackdropBlurVertex> vertices);
Result<void> dx12_record_backdrop_blur(Dx12State& state, const BackdropBlurPlan& plan);
} // namespace cgpui
