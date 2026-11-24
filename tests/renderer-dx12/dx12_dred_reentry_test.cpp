#include "dx12_dred_scalar_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
void inspect(DredStatusScript& api, Dx12DredSitePhase phase) noexcept {
  const auto view = api.site->snapshot_locked();
  api.bad_phase |= view.phase != phase || view.last_is_current;
  api.bad_previous |= view.last_loss.has_value() != api.expect_previous;
  if (view.last_loss) api.bad_previous |= view.last_loss->actual_removed_reason != api.previous_reason;
  api.nested_published |= api.site->observe(api);
}
void reason_reentry(DredStatusScript& api) noexcept {
  api.during_reason = nullptr; // Keep the deliberate RED mutation bounded too.
  inspect(api, Dx12DredSitePhase::checking);
}
void query_reentry(DredStatusScript& api) noexcept {
  api.during_query = nullptr;
  inspect(api, Dx12DredSitePhase::collecting);
}
}
int main() {
  Dx12DredSite site;
  site.configure_setup(Dx12DredSetupRequestReturned{{0, true}});
  site.open_local_lease();
  DredStatusScript api;
  api.site = &site;
  api.reason = dred_removed;
  api.query = {0, true};
  api.during_reason = reason_reentry;
  api.during_query = query_reentry;
  if (!site.observe(api) || api.bad_phase || api.bad_previous || api.nested_published ||
      api.reasons != 1 || api.queries != 1)
    return dred_fail(1, "reason/QI reentry observed partial state or re-probed the current lease");
  site.close_local_lease();
  site.open_local_lease();
  api.reason = dred_reset;
  api.query = {dred_no_interface, false};
  api.expect_previous = true;
  api.previous_reason = dred_removed;
  api.during_reason = reason_reentry;
  api.during_query = query_reentry;
  if (!site.observe(api) || api.bad_phase || api.bad_previous || api.nested_published ||
      api.reasons != 2 || api.queries != 2 ||
      site.snapshot_locked().last_loss->actual_removed_reason != dred_reset)
    return dred_fail(2, "reentry must retain only the previous complete value until publication");
  return 0;
}
