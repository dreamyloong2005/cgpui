#include "../../src/renderer/dx12/dx12_adapter_preference_internal.hpp"

using namespace cgpui;
namespace {
class UnavailableFactory final : public IUnknown {
 public:
  HRESULT result = E_NOINTERFACE;
  unsigned calls = 0;
  bool requested_factory6 = false;
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID requested, void** output) override {
    ++calls;
    requested_factory6 = IsEqualIID(requested, __uuidof(IDXGIFactory6));
    *output = nullptr;
    return result;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { return 1; }
  ULONG STDMETHODCALLTYPE Release() override { return 1; }
};
}
int main() {
  UnavailableFactory source;
  Dx12Ptr<IDXGIAdapter1> adapter;
  for (auto kind : {Dx12AdapterKind::high_performance, Dx12AdapterKind::minimum_power}) {
    for (HRESULT status : {E_NOINTERFACE, E_NOTIMPL, E_OUTOFMEMORY, E_FAIL}) {
      source.result = status;
      const auto before = source.calls;
      if (dx12_enumerate_preferred_adapter(source, kind, 0, adapter) != status ||
          adapter || !source.requested_factory6 || source.calls != before + 1) return 1;
    }
  }
  source.result = S_OK; // A broken success-without-interface is not a usable enumerator.
  if (dx12_enumerate_preferred_adapter(source, Dx12AdapterKind::high_performance, 0, adapter) != E_UNEXPECTED)
    return 2;
  const auto before = source.calls;
  for (auto kind : {Dx12AdapterKind::automatic, Dx12AdapterKind::index, Dx12AdapterKind::warp})
    if (dx12_enumerate_preferred_adapter(source, kind, 0, adapter) != E_INVALIDARG ||
        adapter || source.calls != before) return 3;
  return 0;
}
