#include "dx12_dred_site_internal.hpp"
#include "dx12_dred_api_internal.hpp"

namespace cgpui {
void Dx12DredSite::configure_setup(Dx12DredSetupObservation setup) noexcept {
  if (phase_ == Dx12DredSitePhase::closed) setup_ = setup;
}
void Dx12DredSite::open_local_lease() noexcept {
  if (phase_ == Dx12DredSitePhase::closed) phase_ = Dx12DredSitePhase::open;
}
void Dx12DredSite::close_local_lease() noexcept {
  phase_ = Dx12DredSitePhase::closed;
}
bool Dx12DredSite::observe(Dx12DredStatusApi& api) noexcept {
  if (phase_ != Dx12DredSitePhase::open) return false;
  phase_ = Dx12DredSitePhase::checking;
  const auto reason = api.removed_reason();
  if (reason >= 0) {
    phase_ = Dx12DredSitePhase::open;
    return false;
  }
  phase_ = Dx12DredSitePhase::collecting;
  Dx12DredStatus report;
  report.actual_removed_reason = reason;
  report.setup = setup_;
  if (!std::holds_alternative<Dx12DredNotRequested>(setup_)) {
    report.interface_query = api.query_interface();
    report.breadcrumbs = api.breadcrumb_summary();
    if (report.breadcrumbs.output_requested) report.output = Dx12DredOutput::best_effort_non_atomic;
  }
  last_loss_ = report;
  phase_ = Dx12DredSitePhase::complete;
  return true;
}
Dx12DredSiteSnapshot Dx12DredSite::snapshot_locked() const noexcept {
  return {.phase = phase_, .last_loss = last_loss_,
          .last_is_current = phase_ == Dx12DredSitePhase::complete};
}
} // namespace cgpui
