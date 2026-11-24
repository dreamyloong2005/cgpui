#pragma once
#include "dx12_dred_breadcrumb_model_internal.hpp"
#include "dx12_dred_status_sink_internal.hpp"

namespace cgpui {
Dx12DredStatusMessage dx12_format_dred_breadcrumb_summary(const Dx12DredBreadcrumbSummary&) noexcept;
Dx12DredStatusMessage dx12_format_dred_breadcrumb_node(std::uint32_t, const Dx12DredBreadcrumbNode&) noexcept;
void dx12_emit_dred_breadcrumbs(const Dx12DredBreadcrumbReport&) noexcept;
} // namespace cgpui
