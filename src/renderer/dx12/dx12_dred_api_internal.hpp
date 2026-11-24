#pragma once
#include "dx12_dred_status_internal.hpp"

namespace cgpui {
// External API boundary, borrowed for one synchronous call only.
class Dx12DredSetupApi {
 public:
  virtual ~Dx12DredSetupApi() = default;
  virtual Dx12DredInterfaceResult query_settings() noexcept = 0;
  virtual void enable_breadcrumbs() noexcept = 0;
};
class Dx12DredStatusApi {
 public:
  virtual ~Dx12DredStatusApi() = default;
  virtual std::int32_t removed_reason() noexcept = 0;
  // Implementations release every temporary interface before this returns.
  virtual Dx12DredInterfaceResult query_interface() noexcept = 0;
  virtual Dx12DredBreadcrumbSummary breadcrumb_summary() const noexcept = 0;
};
Dx12DredSetupObservation dx12_request_dred_setup(bool requested,
                                               Dx12DredSetupApi& api) noexcept;
Dx12DredSetupObservation dx12_dred_setup(bool debug_enabled) noexcept;
} // namespace cgpui
