#include "dx12_device_internal.hpp"
#include "dx12_state_internal.hpp"

namespace cgpui {
HRESULT dx12_open_adapter_device(Dx12State& state) {
  state.device.Reset();
  HRESULT result = D3D12CreateDevice(state.adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                    IID_PPV_ARGS(&state.device));
  if (FAILED(result)) return result;
  D3D12_FEATURE_DATA_SHADER_MODEL model{D3D_SHADER_MODEL_6_0};
  result = state.device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &model, sizeof(model));
  if (FAILED(result) || model.HighestShaderModel < D3D_SHADER_MODEL_6_0) {
    state.device.Reset();
    return DXGI_ERROR_UNSUPPORTED;
  }
  return S_OK;
}

Result<void> dx12_select_pinned_adapter(Dx12State& state, std::uint64_t value) {
  const LUID luid{static_cast<DWORD>(value), static_cast<LONG>(value >> 32)};
  HRESULT result = state.factory->EnumAdapterByLuid(luid, IID_PPV_ARGS(&state.adapter));
  if (FAILED(result)) return std::unexpected(dx12_error(result, "pinned adapter lookup"));
  result = dx12_open_adapter_device(state);
  if (FAILED(result)) return std::unexpected(dx12_error(result, "pinned adapter device creation"));
  return {};
}
} // namespace cgpui
