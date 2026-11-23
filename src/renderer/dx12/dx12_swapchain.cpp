#include "dx12_state_internal.hpp"
#include "dx12_composition_internal.hpp"
#include "dx12_window_alpha_internal.hpp"

namespace cgpui {
namespace {
UINT swapchain_flags(const Dx12State& state) {
  return state.composition_surface ? 0 : DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
}
} // namespace

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

Result<void> dx12_create_swapchain(Dx12State& state) {
  if (auto valid = dx12_validate_window_alpha(state); !valid) return valid;
  DXGI_SWAP_CHAIN_DESC1 descriptor{};
  descriptor.Width = state.width;
  descriptor.Height = state.height;
  descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  descriptor.SampleDesc.Count = 1;
  descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  descriptor.BufferCount = static_cast<UINT>(state.targets.size());
  descriptor.SwapEffect = state.composition_surface
      ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_FLIP_DISCARD;
  descriptor.AlphaMode = state.composition_surface
      ? DXGI_ALPHA_MODE_PREMULTIPLIED : DXGI_ALPHA_MODE_IGNORE;
  descriptor.Scaling = DXGI_SCALING_STRETCH;
  descriptor.Flags = swapchain_flags(state);
  Dx12Ptr<IDXGISwapChain1> chain;
  HRESULT result = state.composition_surface
      ? state.factory->CreateSwapChainForComposition(state.queue.Get(), &descriptor, nullptr, &chain)
      : state.factory->CreateSwapChainForHwnd(
          state.queue.Get(), state.hwnd, &descriptor, nullptr, nullptr, &chain);
  if (FAILED(result)) return std::unexpected(dx12_error(result,
      state.composition_surface ? "composition swapchain" : "opaque HWND swapchain"));
  result = chain.As(&state.swapchain);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "swapchain interface"));
  if (!state.composition_surface) {
    result = state.factory->MakeWindowAssociation(state.hwnd, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(result)) return std::unexpected(dx12_error(result, "window association"));
    result = state.swapchain->SetMaximumFrameLatency(1);
    if (FAILED(result)) return std::unexpected(dx12_error(result, "maximum frame latency"));
  }
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heap.NumDescriptors = static_cast<UINT>(state.targets.size());
  result = state.device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&state.rtv_heap));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "RTV heap"));
  state.rtv_stride = state.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  if (auto targets = dx12_create_targets(state); !targets) return targets;
  return state.composition_surface ? dx12_create_composition_host(state) : Result<void>{};
}

Result<void> dx12_resize(Dx12State& state, Size size, DpiScale scale) {
  if (auto valid = dx12_validate_extent(size, scale); !valid) return valid;
  if (state.frame_active) return std::unexpected(Error{.code = ErrorCode::invalid_argument,
      .message = "DX12 cannot resize while a frame is outstanding"});
  if (state.failed) return std::unexpected(dx12_unsupported("recovery of a failed device"));
  if (auto idle = dx12_wait_idle(state); !idle) { state.failed = true; return idle; }
  const auto width = static_cast<UINT>(size.width), height = static_cast<UINT>(size.height);
  state.scale = scale;
  if (width == state.width && height == state.height) return {};
  state.pixels.reset();
  state.readback.Reset();
  for (auto& target : state.targets) target.Reset();
  const HRESULT result = state.swapchain->ResizeBuffers(
      static_cast<UINT>(state.targets.size()), width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
      swapchain_flags(state));
  if (FAILED(result)) { state.failed = true; return std::unexpected(dx12_error(result, "resize buffers")); }
  state.width = width;
  state.height = height;
  auto targets = dx12_create_targets(state);
  if (!targets) state.failed = true;
  return targets;
}
} // namespace cgpui
