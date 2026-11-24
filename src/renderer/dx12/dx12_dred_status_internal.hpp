#pragma once
#include "dx12_dred_breadcrumb_summary_internal.hpp"
#include <cstdint>
#include <variant>

namespace cgpui {
struct Dx12DredInterfaceResult {
  std::int32_t hresult = 0;
  bool received = false;
};
struct Dx12DredNotRequested {};
struct Dx12DredSetupUnavailable { Dx12DredInterfaceResult query; };
// The settings setter returns void, not the HRESULT retained in query.
struct Dx12DredSetupRequestReturned { Dx12DredInterfaceResult query; };
using Dx12DredSetupObservation = std::variant<Dx12DredNotRequested,
    Dx12DredSetupUnavailable, Dx12DredSetupRequestReturned>;
struct Dx12DredDebugNotRequested {};
using Dx12DredQueryObservation = std::variant<Dx12DredDebugNotRequested,
    Dx12DredInterfaceResult>;
enum class Dx12DredScope { site_local };
enum class Dx12DredOutput { not_requested_boundary_unproven, best_effort_non_atomic };
enum class Dx12DredUnknown { unknown };
struct Dx12DredStatus {
  std::int32_t actual_removed_reason = 0;
  Dx12DredSetupObservation setup;
  Dx12DredQueryObservation interface_query;
  Dx12DredScope scope = Dx12DredScope::site_local;
  Dx12DredOutput output = Dx12DredOutput::not_requested_boundary_unproven;
  Dx12DredUnknown breadcrumb_count = Dx12DredUnknown::unknown; // Global total remains unknown.
  Dx12DredBreadcrumbSummary breadcrumbs;
  Dx12DredUnknown native_epoch = Dx12DredUnknown::unknown;
  Dx12DredUnknown participant_coverage = Dx12DredUnknown::unknown;
  Dx12DredUnknown native_device_setup_coverage = Dx12DredUnknown::unknown;
};
} // namespace cgpui
