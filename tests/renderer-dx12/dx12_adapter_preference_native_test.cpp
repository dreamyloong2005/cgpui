#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_native_internal.hpp"
#include <optional>
#include <string>
#include <utility>

using namespace cgpui;
using namespace cgpui_test;
namespace {
struct Environment {
  const char* name;
  std::string old;
  explicit Environment(const char* key) : name(key) {
    if (const auto* value = std::getenv(key)) old = value;
  }
  ~Environment() { _putenv_s(name, old.c_str()); }
};
Result<std::optional<std::uint64_t>> expected_luid(DXGI_GPU_PREFERENCE preference) {
  Dx12Ptr<IDXGIFactory6> factory;
  HRESULT status = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
  if (status == E_NOINTERFACE) return std::optional<std::uint64_t>{};
  if (FAILED(status)) return std::unexpected(dx12_error(status, "native preference oracle factory"));
  for (UINT index = 0;; ++index) {
    Dx12Ptr<IDXGIAdapter1> adapter;
    status = factory->EnumAdapterByGpuPreference(index, preference, IID_PPV_ARGS(&adapter));
    if (status == DXGI_ERROR_NOT_FOUND) return std::optional<std::uint64_t>{};
    if (FAILED(status)) return std::unexpected(dx12_error(status, "native preference oracle enumeration"));
    DXGI_ADAPTER_DESC1 description{};
    status = adapter->GetDesc1(&description);
    if (FAILED(status)) return std::unexpected(dx12_error(status, "native preference oracle description"));
    if (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
    Dx12Ptr<ID3D12Device> device;
    status = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
    if (SUCCEEDED(status)) {
      D3D12_FEATURE_DATA_SHADER_MODEL shader{D3D_SHADER_MODEL_6_0};
      status = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shader, sizeof(shader));
      if (SUCCEEDED(status) && shader.HighestShaderModel < D3D_SHADER_MODEL_6_0) status = DXGI_ERROR_UNSUPPORTED;
      const auto removed = device->GetDeviceRemovedReason();
      if (FAILED(removed)) status = removed;
    }
    if (SUCCEEDED(status)) return std::optional<std::uint64_t>{
        (std::uint64_t(std::uint32_t(description.AdapterLuid.HighPart)) << 32) | description.AdapterLuid.LowPart};
    if (status != DXGI_ERROR_UNSUPPORTED && status != DXGI_ERROR_NOT_FOUND &&
        status != E_NOINTERFACE && status != E_NOTIMPL)
      return std::unexpected(dx12_error(status, "native preference oracle capability"));
  }
}
bool paint(Renderer& renderer) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return false;
  (*frame)->clear({0, 1, 0, 1});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 5, 5, {0, 255, 0, 255});
}
}
int main() {
  Dx12RecoveryDeadline deadline;
  Environment adapter("CGPUI_DX12_ADAPTER"), backend("CGPUI_RENDERER");
  if (!deadline.done || _putenv_s(backend.name, "auto")) return dx12_fail(1, "preference setup failed");
  for (const auto& [text, preference] : {std::pair{"high_performance", DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE},
      {"minimum_power", DXGI_GPU_PREFERENCE_MINIMUM_POWER}}) {
    Dx12Window window;
    if (!window.hwnd || _putenv_s(adapter.name, text)) return dx12_fail(2, "preference HWND/setting failed");
    // Intentionally overrides the enclosing lane's adapter setting. Diagnostics remain inherited.
    auto renderer = create_renderer(window.descriptor());
    const auto expected = expected_luid(preference); // Independent native ordering, after diagnostic setup.
    if (!expected) return dx12_fail(4, expected.error().message.c_str());
    if (!*expected) {
      if (renderer || renderer.error().code != ErrorCode::renderer_initialization_failed ||
          !renderer.error().message.starts_with("DX12 adapter/device selection failed ("))
        return dx12_fail(10, "unavailable native preference did not preserve DX12 failure without fallback");
      std::cout << "adapter preference: selector=" << text << " overrides_lane_adapter=1 hardware_unavailable=1"
                << " renderer_failed=1 fallback=none error=" << renderer.error().message << '\n';
      continue; // Proven unavailable native ordering, not an unobserved hardware skip.
    }
    if (!renderer) return dx12_fail(3, renderer.error().message.c_str());
    const auto before = (*renderer)->backend_info();
    if (before.backend != RendererBackend::dx12 || before.selection != RendererBackendSelection::automatic ||
        before.software_adapter || before.adapter_luid != *expected || !before.fallback_reason.empty())
      return dx12_fail(5, "automatic renderer did not honor native preference order");
    std::cout << "adapter preference: selector=" << text << " overrides_lane_adapter=1 luid=" << **expected
              << " software=0 generation=" << before.device_generation << '\n';
    if (!paint(**renderer)) return dx12_fail(6, "preferred device capture failed");
    if (_putenv_s(adapter.name, "not-an-adapter") || !dx12_remove_native_device(before))
      return dx12_fail(7, "preference recovery preparation failed");
    const auto retry = (*renderer)->begin_frame();
    if (retry || retry.error().code != ErrorCode::frame_retry_required)
      return dx12_fail(8, "preferred device did not recover on its pinned LUID");
    const auto after = (*renderer)->backend_info();
    if (after.adapter_luid != before.adapter_luid || after.device_generation != before.device_generation + 1 ||
        after.backend != before.backend || !after.fallback_reason.empty() || !paint(**renderer) ||
        !dx12_recovered_native_device_clean(after))
      return dx12_fail(9, "preference recovery changed adapter or lost pixels/debug cleanliness");
  }
  return 0;
}
