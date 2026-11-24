#include "dx12_adapter_preference_internal.hpp"

namespace cgpui {
HRESULT dx12_enumerate_preferred_adapter(IUnknown& source, Dx12AdapterKind kind,
                                        UINT index, Dx12Ptr<IDXGIAdapter1>& adapter) {
  adapter.Reset();
  if (kind != Dx12AdapterKind::high_performance && kind != Dx12AdapterKind::minimum_power)
    return E_INVALIDARG;
  Dx12Ptr<IDXGIFactory6> factory;
  const HRESULT queried = source.QueryInterface(IID_PPV_ARGS(&factory));
  if (FAILED(queried)) return queried;
  if (!factory) return E_UNEXPECTED;
  const auto preference = kind == Dx12AdapterKind::high_performance
      ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_MINIMUM_POWER;
  const HRESULT result = factory->EnumAdapterByGpuPreference(index, preference, IID_PPV_ARGS(&adapter));
  if (FAILED(result)) adapter.Reset();
  return SUCCEEDED(result) && !adapter ? E_UNEXPECTED : result;
}
} // namespace cgpui
