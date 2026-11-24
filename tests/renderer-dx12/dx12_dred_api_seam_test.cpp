#include "dx12_dred_scalar_test_support.hpp"
#include "../../src/renderer/dx12/dx12_dred_native_status_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_internal.hpp"
#include <type_traits>

using namespace cgpui;
using namespace cgpui_test;
namespace {
// Fake COM contracts only: these are not a device, native loss, or driver output.
struct DredObject final : ID3D12DeviceRemovedExtendedData {
  ULONG references = 1;
  unsigned adds = 0, releases = 0, output_calls = 0;
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) override {
    if (!result) return E_POINTER;
    *result = nullptr;
    if (!IsEqualIID(iid, __uuidof(IUnknown)) &&
        !IsEqualIID(iid, __uuidof(ID3D12DeviceRemovedExtendedData))) return E_NOINTERFACE;
    *result = static_cast<ID3D12DeviceRemovedExtendedData*>(this);
    AddRef();
    return S_OK;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { ++adds; return ++references; }
  ULONG STDMETHODCALLTYPE Release() override { ++releases; return --references; }
  HRESULT STDMETHODCALLTYPE GetAutoBreadcrumbsOutput(D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT*) override {
    ++output_calls;
    return E_FAIL;
  }
  HRESULT STDMETHODCALLTYPE GetPageFaultAllocationOutput(D3D12_DRED_PAGE_FAULT_OUTPUT*) override {
    ++output_calls;
    return E_FAIL;
  }
};
struct QuerySource final : IUnknown {
  DredObject& object;
  HRESULT result = S_OK;
  bool provide = true, correct_iid = false;
  unsigned queries = 0, adds = 0, releases = 0;
  explicit QuerySource(DredObject& value) : object(value) {}
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** out) override {
    ++queries;
    correct_iid = IsEqualIID(iid, __uuidof(ID3D12DeviceRemovedExtendedData));
    if (!out) return E_POINTER;
    *out = nullptr;
    if (provide) { object.AddRef(); *out = static_cast<ID3D12DeviceRemovedExtendedData*>(&object); }
    return result;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { ++adds; return 1; }
  ULONG STDMETHODCALLTYPE Release() override { ++releases; return 1; }
};
}
static_assert(std::is_trivially_copyable_v<Dx12DredInterfaceResult>);
int main() {
  for (const bool provide : {false, true}) {
    for (const HRESULT hresult : {S_OK, E_NOINTERFACE}) {
      DredObject object;
      QuerySource source(object);
      source.provide = provide;
      source.result = hresult;
      const auto observed = dx12_query_dred_interface(source);
      if (observed.hresult != hresult || observed.received != provide || source.queries != 1 ||
          !source.correct_iid || source.adds || source.releases)
        return dred_fail(1, "native COM seam changed actual result/presence or retained source");
      if (object.references != 1 || object.adds != unsigned(provide) ||
          object.releases != unsigned(provide) || object.output_calls)
        return dred_fail(2, "temporary QI reference escaped or a borrowed output was requested");
    }
  }
  return 0;
}
