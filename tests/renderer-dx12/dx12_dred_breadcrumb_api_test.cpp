#include "dx12_dred_breadcrumb_test_support.hpp"
#include "../../src/renderer/dx12/dx12_dred_breadcrumb_native_internal.hpp"
using namespace cgpui;
using namespace cgpui_test;
namespace {
struct OutputObject final : ID3D12DeviceRemovedExtendedData {
  ULONG references = 1;
  unsigned queries = 0, adds = 0, releases = 0, output_calls = 0, page_fault_calls = 0;
  HRESULT query_result = S_OK, output_result = S_OK;
  bool provide = true;
  const D3D12_AUTO_BREADCRUMB_NODE* head = nullptr;
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** out) override {
    ++queries;
    if (!out) return E_POINTER;
    *out = nullptr;
    if (!IsEqualIID(iid, __uuidof(ID3D12DeviceRemovedExtendedData))) return E_NOINTERFACE;
    if (provide) { *out = static_cast<ID3D12DeviceRemovedExtendedData*>(this); AddRef(); }
    return query_result;
  }
  ULONG STDMETHODCALLTYPE AddRef() override { ++adds; return ++references; }
  ULONG STDMETHODCALLTYPE Release() override { ++releases; return --references; }
  HRESULT STDMETHODCALLTYPE GetAutoBreadcrumbsOutput(D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT* out) override {
    ++output_calls; out->pHeadAutoBreadcrumbNode = head; return output_result;
  }
  HRESULT STDMETHODCALLTYPE GetPageFaultAllocationOutput(D3D12_DRED_PAGE_FAULT_OUTPUT*) override {
    ++page_fault_calls; return E_UNEXPECTED;
  }
};
}
int main() {
  UINT completed = 1;
  const D3D12_AUTO_BREADCRUMB_OP op = D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED;
  D3D12_AUTO_BREADCRUMB_NODE node{};
  node.BreadcrumbCount = 1; node.pLastBreadcrumbValue = &completed; node.pCommandHistory = &op;
  for (bool provide : {false, true}) for (HRESULT query : {S_OK, E_NOINTERFACE}) {
    OutputObject source; source.provide = provide; source.query_result = query; source.head = &node;
    Dx12DredBreadcrumbSummary summary;
    const auto observed = dx12_query_dred_breadcrumbs(source, summary);
    const bool expected = provide && SUCCEEDED(query);
    if (observed.hresult != query || observed.received != provide || source.references != 1 ||
        source.adds != unsigned(provide) || source.releases != unsigned(provide) || source.queries != 1 ||
        source.output_calls != unsigned(expected) || source.page_fault_calls || summary.output_requested != expected)
      return dred_breadcrumb_fail(1, "QI result/presence/reference ownership or output-call gate differs");
    if (expected && (summary.status != Dx12DredBreadcrumbStatus::sampled || summary.nodes != 1 || summary.operations != 1))
      return dred_breadcrumb_fail(2, "native output seam did not use the actual guarded reader");
  }
  OutputObject failed; failed.output_result = DXGI_ERROR_UNSUPPORTED;
  failed.head = reinterpret_cast<const D3D12_AUTO_BREADCRUMB_NODE*>(1);
  Dx12DredBreadcrumbSummary summary;
  (void)dx12_query_dred_breadcrumbs(failed, summary);
  if (!summary.output_requested || summary.output_hresult != DXGI_ERROR_UNSUPPORTED ||
      summary.status != Dx12DredBreadcrumbStatus::query_failed || summary.nodes || failed.references != 1)
    return dred_breadcrumb_fail(3, "failed output query followed poisoned output pointer or lost HRESULT");
  OutputObject empty; (void)dx12_query_dred_breadcrumbs(empty, summary);
  if (summary.status != Dx12DredBreadcrumbStatus::observed_empty || !summary.output_requested || summary.output_hresult != S_OK)
    return dred_breadcrumb_fail(4, "successful empty output confused with query/read failure");
  return 0;
}
