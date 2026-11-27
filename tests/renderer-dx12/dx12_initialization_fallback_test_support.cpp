#include "dx12_initialization_fallback_test_support.hpp"
#include "dx12_ordinary_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"
#include "../../src/renderer/renderer_backend_selection_internal.hpp"
#include "../../src/renderer/dx12/dx12_initialization_attempt_internal.hpp"
#include "../../src/renderer/dx12/dx12_initialization_internal.hpp"
#include "../../src/renderer/dx12/dx12_initialization_rollback_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_pacing_native_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include "../../src/platform/win32_surface_alpha_protocol_internal.hpp"
#include <cstdlib>

namespace cgpui_test {
namespace {
struct Environment {
  const char* name;
  std::string before;
  explicit Environment(const char* key) : name(key) {
    if (const char* old = std::getenv(key)) before = old;
    _putenv_s(key, "");
  }
  ~Environment() { _putenv_s(name, before.c_str()); }
};
class UnavailablePacing final : public cgpui::Dx12FramePacingApi {
 public:
  unsigned sets = 0;
  HRESULT set_maximum_latency(IDXGISwapChain2* chain, UINT) noexcept override {
    ++sets;
    cgpui::Dx12Ptr<ID3D12Device> device;
    cgpui::Dx12Ptr<ID3D12CommandQueue> queue;
    if (FAILED(chain->GetDevice(IID_PPV_ARGS(&device))) &&
        (FAILED(chain->GetDevice(IID_PPV_ARGS(&queue))) || FAILED(queue->GetDevice(IID_PPV_ARGS(&device))))) return E_FAIL;
    const LUID luid = device->GetAdapterLuid();
    const auto value = (std::uint64_t(std::uint32_t(luid.HighPart)) << 32) | luid.LowPart;
    std::cout << "DX12_AUTO_FALLBACK forced_unpinned=1 phase=late-native-failure actual_dx12_luid=" << value << std::endl;
    return DXGI_ERROR_UNSUPPORTED;
  }
  HANDLE latency_object(IDXGISwapChain2*) noexcept override { return nullptr; }
  cgpui::Dx12FramePacingWaitResult poll(HANDLE) noexcept override { return {}; }
  bool close(HANDLE) noexcept override { return false; }
};
UnavailablePacing* pacing = nullptr;
unsigned vulkan_calls = 0, refused = 0;
WNDPROC original = nullptr;
bool block_release = false;
WPARAM release_cookie = 0;
LPARAM release_instance = 0;
LRESULT CALLBACK intercept(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (block_release && message == cgpui::win32_surface_alpha_release_message) {
    ++refused; release_cookie = wparam; release_instance = lparam;
    return E_FAIL; // Leave the real platform owner active, not a fabricated release.
  }
  return CallWindowProcW(original, hwnd, message, wparam, lparam);
}
cgpui::detail::RendererInitializationAttempt attempt(const cgpui::RenderSurfaceDescriptor& descriptor) {
  return cgpui::dx12_attempt_renderer_initialization_with_api(descriptor, *pacing);
}
cgpui::Result<std::unique_ptr<cgpui::Renderer>> vulkan(const cgpui::RenderSurfaceDescriptor& descriptor) {
  ++vulkan_calls;
  return cgpui::create_vulkan_renderer(descriptor);
}
bool capture(cgpui::Renderer& renderer) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({1, 0, 0, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 5, 5, {255, 0, 0, 255});
}
}
int initialization_fallback_case(bool refuse_release, bool full_uncommitted_host, bool expect_loader_failure,
                                  void* desktop_ready) {
  using namespace cgpui;
  // This executable exercises unpinned automatic policy. Enabling DX12-only
  // diagnostics/pinning is separately tested as a hard no-fallback contract.
  Environment adapter("CGPUI_DX12_ADAPTER"), debug("CGPUI_DX12_DEBUG"), gpu("CGPUI_DX12_GPU_VALIDATION");
  std::cout << "DX12_AUTO_FALLBACK forced_unpinned=1 adapter_override=empty debug=0 gpu_validation=0" << std::endl;
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  if (!deadline.done) return 1;
  if (desktop_ready) {
    const bool notified = SetEvent(desktop_ready) != FALSE;
    CloseHandle(desktop_ready);
    if (!notified) return 16;
  }
  OrdinaryDesktop desktop;
  auto app = create_platform_application();
  if (!deadline.done || !desktop.valid() || !app) return 1;
  auto window = (*app)->create_window(desktop.descriptor(true), [](const PlatformEvent&) {});
  if (!window || !ordinary_install_menu(**app, **window) || !desktop.place(**window)) return 2;
  if (const auto gated = ordinary_alpha_os_gate(**window)) return *gated;
  const HWND hwnd = ordinary_hwnd(**window);
  const HMENU menu = GetMenu(hwnd);
  const auto state = (*window)->state();
  const RenderSurfaceDescriptor descriptor{(*window)->native_surface(), state.framebuffer_size, state.scale, true};
  if (full_uncommitted_host) {
    auto native = std::make_shared<Dx12State>();
    const auto identity = dx12_initialization_window(descriptor);
    {
      Dx12OperationGuard guard(*native);
      if (!dx12_initialize_renderer_state(native, descriptor) || !native->composition_host ||
          native->composition_host->committed || native->submitted) return 3;
      auto* host = native->composition_host.get();
      native->submitted = true;
      if (dx12_rollback_initialization(*native, identity) || native->composition_host.get() != host) return 12;
      native->submitted = false;
      host->committed = true;
      if (dx12_rollback_initialization(*native, identity) || native->composition_host.get() != host) return 13;
      host->committed = false;
      auto stale = identity; ++stale.instance;
      if (dx12_rollback_initialization(*native, stale) || native->composition_host.get() != host) return 14;
      if (!dx12_rollback_initialization(*native, identity) || native->composition_host ||
          native->window_alpha || native->device || native->swapchain) return 4;
    }
    const auto dx_info = native->info;
    std::cout << "DX12_AUTO_HOST forced_unpinned=1 actual_dx12_luid=" << dx_info.adapter_luid.value_or(0)
              << " adapter=" << dx_info.adapter_name << std::endl;
    auto renderer = create_vulkan_renderer(descriptor);
    if (!renderer || !capture(**renderer) || ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu) return 5;
    return 0;
  }
  UnavailablePacing api; pacing = &api;
  vulkan_calls = refused = 0; block_release = refuse_release;
  original = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(intercept)));
  if (!original) return 6;
  auto renderer = detail::select_renderer_backend(descriptor, "auto", vulkan, create_dx12_renderer, attempt);
  block_release = false;
  SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original));
  if (api.sets != 1 || ordinary_hwnd(**window) != hwnd || GetMenu(hwnd) != menu) return 7;
  if (refuse_release) {
    if (renderer || vulkan_calls || !refused || renderer.error().message.find("ordinary alpha window lease") == std::string::npos) return 8;
    if (FAILED(static_cast<HRESULT>(SendMessageW(hwnd, win32_surface_alpha_release_message,
        release_cookie, release_instance)))) return 9;
  } else if (expect_loader_failure) {
    if (renderer || vulkan_calls != 1 || refused ||
        renderer.error().code != ErrorCode::renderer_initialization_failed ||
        renderer.error().message.find("maximum frame latency") == std::string::npos ||
        renderer.error().message.find("Vulkan initialization failed") == std::string::npos ||
        renderer.error().message.find("vulkan-1.dll") == std::string::npos || GetModuleHandleW(L"vulkan-1.dll")) return 15;
    std::cout << "LOADER_CHECKED_FALLBACK same_hwnd=1 rollback=checked dx12=unavailable vulkan=loader_failed module=unloaded\n";
  } else {
    if (!renderer || vulkan_calls != 1 || !capture(**renderer)) return 10;
    const auto info = (*renderer)->backend_info();
    std::cout << "DX12_AUTO_FALLBACK forced_unpinned=1 actual_backend=vulkan adapter=" << info.adapter_name
              << " luid=" << info.adapter_luid.value_or(0) << " generation=" << info.device_generation << std::endl;
    if (info.backend != RendererBackend::vulkan || info.selection != RendererBackendSelection::automatic ||
        info.fallback_reason.find("maximum frame latency") == std::string::npos) return 11;
  }
  return 0;
}
} // namespace cgpui_test
