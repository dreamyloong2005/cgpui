#include "dx12_swapchain_candidate_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_swapchain_flags_internal.hpp"
#include <utility>

namespace cgpui {
Result<std::unique_ptr<Dx12SwapchainCandidate>> dx12_prepare_composition_chain(Dx12State& state, bool premultiplied) {
  auto candidate = std::make_unique<Dx12SwapchainCandidate>();
  DXGI_SWAP_CHAIN_DESC1 descriptor{};
  descriptor.Width = state.width; descriptor.Height = state.height;
  descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  descriptor.SampleDesc.Count = 1;
  descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  descriptor.BufferCount = static_cast<UINT>(candidate->targets.size());
  descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  descriptor.AlphaMode = premultiplied ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;
  descriptor.Scaling = DXGI_SCALING_STRETCH;
  descriptor.Flags = dx12_swapchain_flags();
  Dx12Ptr<IDXGISwapChain1> chain;
  HRESULT result = state.factory->CreateSwapChainForComposition(state.queue.Get(), &descriptor, nullptr, &chain);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "replacement composition swapchain"));
  result = chain.As(&candidate->swapchain);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "replacement swapchain interface"));
  candidate->pacing = std::make_unique<Dx12FramePacing>(state.frame_pacing->api());
  if (auto pacing = candidate->pacing->configure(candidate->swapchain.Get(), true); !pacing)
    return std::unexpected(pacing.error());
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; heap.NumDescriptors = descriptor.BufferCount;
  result = state.device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&candidate->rtv_heap));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "replacement RTV heap"));
  candidate->rtv_stride = state.device->GetDescriptorHandleIncrementSize(heap.Type);
  auto handle = candidate->rtv_heap->GetCPUDescriptorHandleForHeapStart();
  for (UINT index = 0; index != candidate->targets.size(); ++index) {
    result = candidate->swapchain->GetBuffer(index, IID_PPV_ARGS(&candidate->targets[index]));
    if (FAILED(result)) return std::unexpected(dx12_error(result, "replacement swapchain buffer"));
    state.device->CreateRenderTargetView(candidate->targets[index].Get(), nullptr, handle);
    handle.ptr += candidate->rtv_stride;
  }
  return candidate;
}

void dx12_exchange_swapchain(Dx12State& state, Dx12SwapchainCandidate& candidate) noexcept {
  std::swap(state.swapchain, candidate.swapchain);
  std::swap(state.rtv_heap, candidate.rtv_heap);
  std::swap(state.targets, candidate.targets);
  std::swap(state.rtv_stride, candidate.rtv_stride);
  std::swap(state.frame_pacing, candidate.pacing);
}
} // namespace cgpui
