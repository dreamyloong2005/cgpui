#include "dx12_recovery_test_support.hpp"

#include <optional>
#include <string>

using namespace cgpui;
using namespace cgpui_test;
using Microsoft::WRL::ComPtr;

namespace {
Error probe_error(const char* operation, HRESULT result) {
  return {ErrorCode::renderer_initialization_failed,
      std::string(operation) + " failed: " + std::to_string(static_cast<long>(result))};
}
Result<std::optional<std::string>> independent_adapter(const RendererBackendInfo& current) {
  ComPtr<IDXGIFactory4> factory;
  const auto created = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
  if (FAILED(created)) return std::unexpected(probe_error("independent DXGI factory", created));
  for (UINT index = 0;; ++index) {
    ComPtr<IDXGIAdapter1> adapter;
    const HRESULT found = current.software_adapter
        ? factory->EnumAdapters1(index, &adapter)
        : factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
    if (found == DXGI_ERROR_NOT_FOUND) return std::optional<std::string>{};
    if (FAILED(found)) return std::unexpected(probe_error("independent adapter enumeration", found));
    DXGI_ADAPTER_DESC1 description{};
    const auto described = adapter->GetDesc1(&description);
    if (FAILED(described)) return std::unexpected(probe_error("independent adapter description", described));
    const auto luid = (std::uint64_t(std::uint32_t(description.AdapterLuid.HighPart)) << 32) |
        description.AdapterLuid.LowPart;
    if ((!current.software_adapter || !(description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) &&
        current.adapter_luid != luid) {
      ComPtr<ID3D12Device> device;
      const auto opened = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
      if (FAILED(opened) && opened != DXGI_ERROR_UNSUPPORTED)
        return std::unexpected(probe_error("independent adapter device", opened));
      if (SUCCEEDED(opened)) {
        D3D12_FEATURE_DATA_SHADER_MODEL model{D3D_SHADER_MODEL_6_0};
        const auto supported = device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model));
        if (FAILED(supported) && supported != E_INVALIDARG && supported != DXGI_ERROR_UNSUPPORTED)
          return std::unexpected(probe_error("independent adapter shader model", supported));
        if (SUCCEEDED(supported) && model.HighestShaderModel >= D3D_SHADER_MODEL_6_0)
          return std::optional<std::string>{current.software_adapter ? std::to_string(index) : "warp"};
      }
    }
    if (!current.software_adapter) return std::optional<std::string>{};
  }
}
struct AdapterEnvironment {
  std::string saved;
  AdapterEnvironment() { if (const auto* value = std::getenv("CGPUI_DX12_ADAPTER")) saved = value; }
  ~AdapterEnvironment() { _putenv_s("CGPUI_DX12_ADAPTER", saved.c_str()); }
};
Result<std::unique_ptr<Renderer>> create_independent(Dx12Window& window, const std::string& selection) {
  AdapterEnvironment restore;
  if (_putenv_s("CGPUI_DX12_ADAPTER", selection.c_str()) != 0)
    return std::unexpected(Error{ErrorCode::invalid_argument, "cannot set test adapter preference"});
  return create_dx12_renderer(window.descriptor());
}
bool paint(Renderer& renderer, Color color, std::array<std::uint8_t, 4> expected) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear(color);
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, expected);
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window first_window, independent_window;
  if (!deadline.done || !first_window.hwnd || !independent_window.hwnd)
    return dx12_fail(1, "adapter isolation fixture unavailable");
  auto first = create_dx12_renderer(first_window.descriptor());
  if (!first) return dx12_fail(2, first.error().message.c_str());
  const auto first_before = (*first)->backend_info();
  const auto selection = independent_adapter(first_before);
  if (!selection) return dx12_fail(3, selection.error().message.c_str());
  if (!*selection) {
    std::cout << "SKIP: no independent SM6 adapter; cross-adapter isolation was not tested\n";
    return 0;
  }
  auto independent = create_independent(independent_window, **selection);
  if (!independent) return dx12_fail(3, independent.error().message.c_str());
  const auto independent_before = (*independent)->backend_info();
  if (!first_before.adapter_luid || !independent_before.adapter_luid ||
      first_before.adapter_luid == independent_before.adapter_luid)
    return dx12_fail(4, "adapter isolation fixture did not create distinct LUIDs");
  if (!paint(**first, {1, 0, 0, 1}, {255, 0, 0, 255}) ||
      !paint(**independent, {0, 1, 0, 1}, {0, 255, 0, 255}))
    return dx12_fail(5, "independent adapter warmup failed");
  const auto* borrowed = (*independent)->last_frame_pixels();
  const auto saved = borrowed->rgba8;
  if (!dx12_remove_native_device(first_before)) return dx12_fail(6, "isolated native removal failed");
  const auto retry = (*first)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      (*first)->backend_info().device_generation != first_before.device_generation + 1 ||
      (*first)->backend_info().adapter_luid != first_before.adapter_luid)
    return dx12_fail(7, "isolated initiator recovery failed");
  const auto unchanged = (*independent)->backend_info();
  if (unchanged.device_generation != independent_before.device_generation ||
      unchanged.adapter_luid != independent_before.adapter_luid ||
      (*independent)->last_frame_pixels() != borrowed || borrowed->rgba8 != saved)
    return dx12_fail(8, "unrelated adapter generation or borrowed capture was changed");
  if (!paint(**independent, {0, 0, 1, 1}, {0, 0, 255, 255}) ||
      (*independent)->backend_info().device_generation != independent_before.device_generation)
    return dx12_fail(9, "unrelated adapter requested recovery or failed its next frame");
  if (!paint(**first, {1, 0, 0, 1}, {255, 0, 0, 255}) ||
      !dx12_recovered_native_device_clean((*first)->backend_info()) ||
      !dx12_recovered_native_device_clean(independent_before))
    return dx12_fail(10, "isolated devices failed rendering or debug validation");
  std::cout << "Cross-adapter isolation exercised distinct hardware/WARP LUIDs\n";
  return 0;
}
