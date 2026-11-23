#include "dx12_composition_probe_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_surface_internal.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include <cstdio>
#include <iomanip>

namespace cgpui_test {
Dx12CompositionProbe::Dx12CompositionProbe() = default;
Dx12CompositionProbe::~Dx12CompositionProbe() { state.reset(); }
bool Dx12CompositionProbe::initialize(bool visible) {
  using namespace cgpui;
  visible_ = visible;
  composition_probe_note("mode", visible ? "visible; experimental-only" : "initially-hidden; never-show");
  if (!windows.open(visible)) {
    composition_probe_note("window_setup", "unmeasured: desktop/DPI/native HWND fixture unavailable");
    return false;
  }
  Dx12Ptr<ID3D12Debug> debug;
  if (!native("debug_interface", D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) return false;
  debug->EnableDebugLayer();
  debug.Reset();
  state = std::make_shared<Dx12State>(&api); // This state's sole handle/grant owner uses our forwarder.
  const RenderSurfaceDescriptor surface{
      .native_surface = Win32SurfaceHandle{.hinstance = GetModuleHandleW(nullptr),
          .hwnd = windows.popup(), .composition = true},
      .framebuffer_size = {64, 64}, .scale = {1}, .transparent_background = true};
  if (!result("configure_surface", dx12_configure_surface(*state, surface)) ||
      !result("create_device", dx12_create_device(*state))) return false;
  const auto& info = state->info;
  DXGI_ADAPTER_DESC1 adapter{};
  LARGE_INTEGER version{};
  if (!native("adapter_descriptor", state->adapter->GetDesc1(&adapter))) return false;
  const auto driver = state->adapter->CheckInterfaceSupport(__uuidof(IDXGIDevice), &version);
  composition_probe_hresult("driver_version_query_optional", driver);
  std::cout << "COMPOSITION_PROBE adapter=" << std::quoted(info.adapter_name)
            << " luid=" << info.adapter_luid.value_or(0) << " software=" << info.software_adapter
            << " vendor=" << adapter.VendorId << " device=" << adapter.DeviceId
            << " driver_raw=" << (SUCCEEDED(driver) ? version.QuadPart : 0)
            << " pointer_bits=" << sizeof(void*) * 8 << std::endl;
  if (!invariant(info.backend == RendererBackend::dx12 && info.adapter_luid &&
      info.selection == RendererBackendSelection::dx12 && info.fallback_reason.empty(),
      "actual backend selection differs")) return false;
  return create_chain() && debug_clean() && invariant(windows.visibility_unchanged(), "visibility changed");
}
bool Dx12CompositionProbe::create_chain() {
  using namespace cgpui;
  DXGI_SWAP_CHAIN_DESC1 desc{};
  desc.Width = state->width; desc.Height = state->height;
  desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; desc.SampleDesc.Count = 1;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; desc.BufferCount = 2;
  desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
  desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
  desc.Scaling = DXGI_SCALING_STRETCH; desc.Flags = creation_flags_;
  std::printf("COMPOSITION_PROBE descriptor width=%u height=%u format=%u count=%u effect=%u alpha=%u scaling=%u flags=%u\n",
      desc.Width, desc.Height, desc.Format, desc.BufferCount, desc.SwapEffect, desc.AlphaMode, desc.Scaling, desc.Flags);
  std::fflush(stdout);
  Dx12Ptr<IDXGISwapChain1> chain;
  if (!native("CreateSwapChainForComposition",
      state->factory->CreateSwapChainForComposition(state->queue.Get(), &desc, nullptr, &chain)) ||
      !native("swapchain3_interface", chain.As(&state->swapchain)) ||
      !result("configure_pacing", state->frame_pacing->configure(state->swapchain.Get(), true))) return false;
  DXGI_SWAP_CHAIN_DESC1 actual{};
  UINT maximum = 0;
  if (!native("GetDesc1", state->swapchain->GetDesc1(&actual)) ||
      !native("GetMaximumFrameLatency", state->swapchain->GetMaximumFrameLatency(&maximum)) ||
      !invariant(actual.Flags == creation_flags_ && maximum == 1 &&
          api.sets == 1 && api.gets == 1 && api.acquired == 1, "flag/configuration differs")) return false;
  D3D12_DESCRIPTOR_HEAP_DESC heap{};
  heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV; heap.NumDescriptors = 2;
  if (!native("RTV_heap", state->device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&state->rtv_heap))))
    return false;
  state->rtv_stride = state->device->GetDescriptorHandleIncrementSize(heap.Type);
  return result("create_targets", dx12_create_targets(*state)) && bind_composition();
}
bool Dx12CompositionProbe::bind_composition() {
  using namespace cgpui;
  state->composition_host = std::make_unique<Dx12CompositionHost>();
  auto& host = *state->composition_host;
  if (!native("DCompositionCreateDevice", DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&host.device))) ||
      !native("CreateTargetForHwnd", host.device->CreateTargetForHwnd(state->hwnd, TRUE, &host.target)) ||
      !native("CreateVisual", host.device->CreateVisual(&host.visual)) ||
      !native("SetContent", host.visual->SetContent(state->swapchain.Get())) ||
      !native("SetRoot", host.target->SetRoot(host.visual.Get()))) return false;
  composition_probe_note("composition_bound", "NOT committed; first native wait occurs before draw/Present/Commit");
  return true;
}
bool Dx12CompositionProbe::debug_clean() {
  cgpui::Dx12Ptr<ID3D12InfoQueue> messages;
  return invariant(state && state->device && SUCCEEDED(state->device.As(&messages)) && messages &&
      dx12_recovery_debug_messages_safe(messages.Get()), "InfoQueue unavailable or contains error/corruption");
}
bool Dx12CompositionProbe::close_and_verify() {
  // State destructor retains submitted work until actual fence/removal proof.
  state.reset();
  std::printf("COMPOSITION_PROBE counts sets=%u gets=%u polls=%u ready=%u pending=%u acquired=%u closes=%u\n",
      api.sets, api.gets, api.polls, api.ready, api.pending, api.acquired, api.closes);
  return invariant(api.closes == api.acquired && api.close_ok, "owned handle close accounting differs");
}
} // namespace cgpui_test
