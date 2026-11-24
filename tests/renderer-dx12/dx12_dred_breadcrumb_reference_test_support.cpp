#include "dx12_dred_native_test_support.hpp"
namespace cgpui_test {
void dred_observe_breadcrumb_reference(ID3D12Device& source, DredNativeLoss& loss) {
  Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedData> dred;
  if (FAILED(source.QueryInterface(IID_PPV_ARGS(&dred))) || !dred) return;
  D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT output{};
  loss.breadcrumb_hresult = dred->GetAutoBreadcrumbsOutput(&output);
  loss.breadcrumb_requested = true;
  // Independent actual query/HRESULT only. Never dereference borrowed output.
}
} // namespace cgpui_test
