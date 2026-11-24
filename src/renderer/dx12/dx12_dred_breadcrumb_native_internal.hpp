#pragma once
#include "dx12_dred_breadcrumb_summary_internal.hpp"
#include "dx12_dred_native_status_internal.hpp"

namespace cgpui {
Dx12DredInterfaceResult dx12_query_dred_breadcrumbs(
    IUnknown&, Dx12DredBreadcrumbSummary&) noexcept;
} // namespace cgpui
