#include "dx12_command_bundle_internal.hpp"
#include "dx12_command_bundle_geometry_internal.hpp"
#include "dx12_command_observation_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_frame_diagnostics_internal.hpp"

namespace cgpui {
struct Dx12CommandBundle::Impl {
  Dx12State* state;
  Dx12ReuseScene scene;
  std::unique_ptr<Dx12CommandBundleGeometry> geometry;
  Dx12Ptr<ID3D12CommandAllocator> allocator;
  Dx12Ptr<ID3D12GraphicsCommandList> list;
  Dx12Ptr<ID3D12PipelineState> rect_pipeline, image_pipeline;
  Dx12Ptr<ID3D12RootSignature> rect_root, image_root;
  Dx12Ptr<ID3D12DescriptorHeap> srv_heap, samplers;
  std::vector<Dx12Ptr<ID3D12Resource>> images;
  std::vector<Dx12BundleReceipt> receipts;
};
Dx12CommandBundle::Dx12CommandBundle(Dx12State& state, Dx12ReuseScene scene) : impl_(std::make_unique<Impl>()) {
  impl_->state = &state; impl_->scene = std::move(scene);
  impl_->geometry = std::make_unique<Dx12CommandBundleGeometry>();
}
Dx12CommandBundle::~Dx12CommandBundle() {
  auto* list = impl_->list.Get(); auto* allocator = impl_->allocator.Get();
  impl_->list.Reset(); impl_->allocator.Reset();
  if (list) dx12_observe_command(*impl_->state, {.kind = Dx12CommandEventKind::bundle_released,
      .object = list, .object2 = allocator, .type = D3D12_COMMAND_LIST_TYPE_BUNDLE});
  impl_->geometry.reset();
  dx12_observe_command(*impl_->state, {.kind = Dx12CommandEventKind::frozen_released});
  impl_->images.clear(); impl_->srv_heap.Reset(); impl_->samplers.Reset();
  impl_->rect_pipeline.Reset(); impl_->image_pipeline.Reset(); impl_->rect_root.Reset(); impl_->image_root.Reset();
  impl_->scene.pins.clear(); // Last: release the CPU pins protecting native map nodes AND sole SRV leases.
}
Result<std::unique_ptr<Dx12CommandBundle>> Dx12CommandBundle::create(
    Dx12State& state, Dx12ReuseScene scene, std::span<const Dx12DrawCommand> draws) {
  auto result = std::unique_ptr<Dx12CommandBundle>(new Dx12CommandBundle(state, std::move(scene)));
  if (auto recorded = result->record(state, draws); !recorded) return std::unexpected(recorded.error());
  return result;
}
Result<void> Dx12CommandBundle::record(Dx12State& state, std::span<const Dx12DrawCommand> draws) {
  if (auto packed = impl_->geometry->prepare(*state.device.Get(), draws); !packed) return packed;
  impl_->receipts.reserve(512); impl_->images.reserve(impl_->scene.pins.size());
  impl_->rect_pipeline = state.pipeline; impl_->rect_root = state.root_signature;
  impl_->srv_heap = impl_->scene.heap; impl_->samplers = impl_->scene.samplers;
  for (const auto& binding : impl_->scene.bindings) if (binding.resource) {
    impl_->images.emplace_back(binding.resource);
    impl_->image_pipeline = binding.pipeline; impl_->image_root = binding.root;
  }
  HRESULT result = state.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(&impl_->allocator));
  if (SUCCEEDED(result)) result = state.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE,
      impl_->allocator.Get(), impl_->rect_pipeline.Get(), IID_PPV_ARGS(&impl_->list));
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::bundle_created, .list = impl_->list.Get(),
      .allocator = impl_->allocator.Get(), .type = D3D12_COMMAND_LIST_TYPE_BUNDLE, .result = result});
  if (FAILED(result)) return std::unexpected(dx12_error(result, "immutable bundle create"));
  for (std::size_t group = 0; group < draws.size(); ++group) {
    auto prepared = impl_->geometry->views(group);
    prepared.extent = {float(state.width), float(state.height)};
    if (const auto* geometry = std::get_if<RectGeometry>(&draws[group].payload)) {
      prepared.pipeline = impl_->rect_pipeline.Get(); prepared.root = impl_->rect_root.Get();
      for (std::size_t local = 0; local < geometry->draws.size(); ++local) {
        const auto& range = geometry->draws[local];
        prepared.count = static_cast<UINT>(range.index_count); prepared.first = static_cast<UINT>(range.first_index);
        dx12_emit_prepared_draw(state, *impl_->list.Get(), prepared);
        impl_->receipts.push_back({group, local}); // Capacity fixed before native recording.
      }
    } else {
      const auto& b = impl_->scene.bindings[group];
      prepared.pipeline = b.pipeline; prepared.root = b.root;
      prepared.srv_heap = b.heap; prepared.sampler_heap = b.samplers;
      prepared.srv = b.srv; prepared.sampler = b.sampler;
      dx12_emit_prepared_draw(state, *impl_->list.Get(), prepared);
      impl_->receipts.push_back({group, 0});
    }
  }
  result = impl_->list->Close();
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::bundle_closed, .list = impl_->list.Get(),
      .allocator = impl_->allocator.Get(), .result = result});
  if (FAILED(result)) return std::unexpected(dx12_error(result, "immutable bundle Close"));
  return {};
}
const Dx12ReuseScene& Dx12CommandBundle::scene() const noexcept { return impl_->scene; }
void Dx12CommandBundle::execute(Dx12State& state) {
  state.commands->RSSetScissorRects(1, &impl_->scene.scissor);
  if (impl_->srv_heap) {
    ID3D12DescriptorHeap* heaps[] = {impl_->srv_heap.Get(), impl_->samplers.Get()};
    state.commands->SetDescriptorHeaps(2, heaps);
    dx12_observe_command(state, {.kind = Dx12CommandEventKind::heaps, .list = state.commands.Get(),
        .object = heaps[0], .object2 = heaps[1]});
  }
  state.commands->ExecuteBundle(impl_->list.Get());
  dx12_observe_command(state, {.kind = Dx12CommandEventKind::execute_bundle,
      .list = state.commands.Get(), .object = impl_->list.Get()});
  if (state.active_diagnostics) for (const auto& receipt : impl_->receipts) {
    state.active_diagnostics->begin_draw(receipt.group); state.active_diagnostics->submitted(receipt.local);
  }
}
} // namespace cgpui
