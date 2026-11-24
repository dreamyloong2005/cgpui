#include "dx12_dred_breadcrumb_native_internal.hpp"
#include "dx12_dred_breadcrumb_format_internal.hpp"
#include "dx12_dred_memory_reader_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
void dx12_emit_dred_breadcrumbs(const Dx12DredBreadcrumbReport& report) noexcept {
  const auto header = dx12_format_dred_breadcrumb_summary(report.summary);
  OutputDebugStringW(header.text.data());
  for (std::uint32_t index = 0; index != report.summary.nodes; ++index) {
    const auto text = dx12_format_dred_breadcrumb_node(index, report.nodes[index]);
    OutputDebugStringW(text.text.data());
  }
}
Dx12DredInterfaceResult dx12_query_dred_breadcrumbs(
    IUnknown& source, Dx12DredBreadcrumbSummary& summary) noexcept {
  summary = {};
  Dx12Ptr<ID3D12DeviceRemovedExtendedData> dred;
  const HRESULT queried = source.QueryInterface(IID_PPV_ARGS(&dred));
  const Dx12DredInterfaceResult query_result{static_cast<std::int32_t>(queried), dred.Get() != nullptr};
  if (FAILED(queried) || !dred) return query_result;
  D3D12_DRED_AUTO_BREADCRUMBS_OUTPUT output{};
  const HRESULT result = dred->GetAutoBreadcrumbsOutput(&output);
  Dx12DredBreadcrumbReport report;
  if (SUCCEEDED(result)) report = dx12_sample_dred_breadcrumbs(
      reinterpret_cast<std::uintptr_t>(output.pHeadAutoBreadcrumbNode), dx12_dred_process_reader());
  else report.summary.status = Dx12DredBreadcrumbStatus::query_failed;
  report.summary.output_requested = true;
  report.summary.output_hresult = static_cast<std::int32_t>(result);
  summary = report.summary;
  dx12_emit_dred_breadcrumbs(report);
  return query_result; // Every temporary QI ref is released before returning owned values.
}
} // namespace cgpui
