#include "dx12_swapchain_internal.hpp"
#include "dx12_state_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_window_alpha_internal.hpp"
#include "dx12_frame_pacing_internal.hpp"
#include "dx12_swapchain_flags_internal.hpp"

namespace cgpui {

Result<void> dx12_create_targets(Dx12State& state) {
  auto handle = state.rtv_heap->GetCPUDescriptorHandleForHeapStart();
  for (UINT index = 0; index != state.targets.size(); ++index) {
    const HRESULT result = state.swapchain->GetBuffer(index, IID_PPV_ARGS(&state.targets[index]));
    if (FAILED(result)) return std::unexpected(dx12_error(result, "swapchain buffer"));
    state.device->CreateRenderTargetView(state.targets[index].Get(), nullptr, handle);
    handle.ptr += state.rtv_stride;
  }
  return {};
}

Result<void> dx12_create_swapchain(Dx12State& state, Dx12InitializationFailureClass* failure) {
  if (failure) *failure = Dx12InitializationFailureClass::terminal;
  if (auto valid = dx12_validate_window_alpha(state); !valid) return valid;
  DXGI_SWAP_CHAIN_DESC1 descriptor{};
  descriptor.Width = state.width;
  descriptor.Height = state.height;
  descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  descriptor.SampleDesc.Count = 1;
  descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  descriptor.BufferCount = static_cast<UINT>(state.targets.size());
  descriptor.SwapEffect = dx12_composition_route(state.surface_policy)
      ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_FLIP_DISCARD;
  descriptor.AlphaMode = state.composition_surface
      ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;
  descriptor.Scaling = DXGI_SCALING_STRETCH;
  descriptor.Flags = dx12_swapchain_flags();
  Dx12Ptr<IDXGISwapChain1> chain;
  HRESULT result = dx12_composition_route(state.surface_policy)
      ? state.factory->CreateSwapChainForComposition(state.queue.Get(), &descriptor, nullptr, &chain)
      : state.factory->CreateSwapChainForHwnd(
          state.queue.Get(), state.hwnd, &descriptor, nullptr, nullptr, &chain);
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result,
      dx12_composition_route(state.surface_policy) ? "composition swapchain" : "opaque HWND swapchain", failure));
  result = chain.As(&state.swapchain);
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "swapchain interface", failure));
  if (!dx12_composition_route(state.surface_policy)) {
    result = state.factory->MakeWindowAssociation(state.hwnd, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(result)) return std::unexpected(dx12_error(result, "window association"));
  }
  const bool waitable = (descriptor.Flags & DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT) != 0;
  if (auto pacing = state.frame_pacing->configure(state.swapchain.Get(), waitable, failure); !pacing)
    return pacing;
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heap.NumDescriptors = static_cast<UINT>(state.targets.size());
  result = state.device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&state.rtv_heap));
  if (FAILED(result)) return std::unexpected(dx12_initialization_error(result, "RTV heap", failure));
  state.rtv_stride = state.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  if (auto targets = dx12_create_targets(state); !targets) return targets;
  return dx12_composition_route(state.surface_policy) ? dx12_create_composition_host(state, failure) : Result<void>{};
}

} // namespace cgpui
