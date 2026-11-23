#include "dx12_pacing_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_retirement_internal.hpp"
#include <d3d12sdklayers.h>
#include <cstdlib>
#include <string_view>

namespace cgpui_test {
Dx12PacingNativeFixture::Dx12PacingNativeFixture() : pacing(api) {}
Dx12PacingNativeFixture::~Dx12PacingNativeFixture() {
  if (state) cgpui::dx12_retire_submitted_work(*state);
  pacing.reset(); // Completion/removal or never-submitted proof precedes closure.
  state.reset();
}
bool Dx12PacingNativeFixture::initialize() {
  using namespace cgpui;
  // Each standalone native test enables debug before its first device. Missing
  // debug components fail instead of silently omitting mandatory InfoQueue checks.
  Dx12Ptr<ID3D12Debug> debug;
  if (FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) return false;
  debug->EnableDebugLayer();
  debug.Reset();
  state = std::make_shared<Dx12State>();
  if (!window.hwnd || !dx12_configure_surface(*state, window.descriptor()) ||
      !dx12_create_device(*state) || !dx12_create_pipeline(*state)) return false;
  const auto& info = state->info;
  const char* requested = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = requested && std::string_view(requested) == "warp";
  std::cout << "PACING_NATIVE actual_backend=" << static_cast<int>(info.backend)
            << " adapter=" << info.adapter_name << " software=" << info.software_adapter
            << " luid=" << info.adapter_luid.value_or(0) << '\n';
  if (info.backend != RendererBackend::dx12 || !info.adapter_luid ||
      info.selection != RendererBackendSelection::dx12 || !info.fallback_reason.empty() ||
      (warp && !info.software_adapter)) return false;
  // A visible, nonactivating fixture avoids treating occlusion signaling as a contract.
  ShowWindow(window.hwnd, SW_SHOWNOACTIVATE);
  return create_chain() && debug_clean();
}
bool Dx12PacingNativeFixture::create_chain() {
  using namespace cgpui;
  // Do not call dx12_create_swapchain: stage A does not alter its existing SetMax.
  // This real chain is configured exactly once by the new owner below.
  DXGI_SWAP_CHAIN_DESC1 descriptor{};
  descriptor.Width = state->width;
  descriptor.Height = state->height;
  descriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  descriptor.SampleDesc.Count = 1;
  descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  descriptor.BufferCount = 2;
  descriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  descriptor.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
  descriptor.Scaling = DXGI_SCALING_STRETCH;
  descriptor.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
  Dx12Ptr<IDXGISwapChain1> chain;
  if (FAILED(state->factory->CreateSwapChainForHwnd(state->queue.Get(), window.hwnd,
      &descriptor, nullptr, nullptr, &chain)) || FAILED(chain.As(&state->swapchain)) ||
      FAILED(state->factory->MakeWindowAssociation(window.hwnd, DXGI_MWA_NO_ALT_ENTER))) return false;
  if (!pacing.configure(state->swapchain.Get(), true) || !api.observed_handle) return false;
  UINT maximum = 0;
  if (FAILED(state->swapchain->GetMaximumFrameLatency(&maximum)) || maximum != 1) return false;
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
  heap.NumDescriptors = 2;
  if (FAILED(state->device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&state->rtv_heap)))) return false;
  state->rtv_stride = state->device->GetDescriptorHandleIncrementSize(heap.Type);
  return bool(dx12_create_targets(*state));
}
} // namespace cgpui_test
