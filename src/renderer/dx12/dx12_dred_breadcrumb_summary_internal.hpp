#pragma once
#include <cstdint>

namespace cgpui {
enum class Dx12DredBreadcrumbStatus {
  not_attempted, query_failed, observed_empty, sampled, read_failed,
  short_read, invalid_address, inconsistent, cycle, truncated, budget_exhausted
};
// Owned summary of a bounded non-atomic sample, never a device-wide snapshot.
struct Dx12DredBreadcrumbSummary {
  Dx12DredBreadcrumbStatus status = Dx12DredBreadcrumbStatus::not_attempted;
  std::int32_t output_hresult = 0;
  bool output_requested = false, history_truncated = false;
  std::uint32_t nodes = 0, operations = 0, read_error = 0;
};
const wchar_t* dx12_dred_breadcrumb_status_name(Dx12DredBreadcrumbStatus) noexcept;
} // namespace cgpui
