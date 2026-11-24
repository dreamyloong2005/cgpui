#include "dx12_dred_scalar_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12DredSite site;
  // Debug requested, but the settings acquisition failed. QI must still run.
  site.configure_setup(Dx12DredSetupUnavailable{{dred_no_interface, false}});
  site.open_local_lease();
  DredStatusScript shared;
  shared.reason = dred_removed;
  shared.query = {dred_no_interface, false};
  if (!site.observe(shared)) return dred_fail(1, "first failed lease did not publish");
  const auto prior = site.snapshot_locked();
  const auto* first = std::get_if<Dx12DredInterfaceResult>(&prior.last_loss->interface_query);
  if (!first || first->hresult != dred_no_interface || first->received || shared.queries != 1 ||
      site.observe(shared) || shared.queries != 1 || shared.reasons != 1)
    return dred_fail(2, "failed settings must not skip QI; failed QI must be terminal");
  site.close_local_lease();
  site.open_local_lease();
  const auto reopened = site.snapshot_locked();
  if (reopened.phase != Dx12DredSitePhase::open || reopened.last_is_current || !reopened.last_loss ||
      reopened.last_loss->actual_removed_reason != dred_removed)
    return dred_fail(3, "new lease cleared history or mislabelled prior loss as current");
  shared.reason = dred_reset;
  shared.query = {0, false};
  if (!site.observe(shared) || shared.queries != 2 || shared.reasons != 2)
    return dred_fail(4, "new local lease inherited collection suppression");
  const auto current = site.snapshot_locked();
  const auto* second = std::get_if<Dx12DredInterfaceResult>(&current.last_loss->interface_query);
  if (!second || second->hresult != 0 || second->received || !current.last_is_current ||
      current.last_loss->actual_removed_reason != dred_reset ||
      prior.last_loss->actual_removed_reason != dred_removed)
    return dred_fail(5, "success/null or independent prior owned value was lost");
  Dx12DredSite another;
  another.configure_setup(Dx12DredSetupRequestReturned{{0, true}});
  another.open_local_lease();
  shared.query = {0, true};
  if (!another.observe(shared) || shared.queries != 3 || site.observe(shared) || shared.queries != 3)
    return dred_fail(6, "two sites must remain independently once, not falsely device-deduplicated");
  const auto other = another.snapshot_locked();
  const auto* success = std::get_if<Dx12DredInterfaceResult>(&other.last_loss->interface_query);
  if (!success || !success->received || success->hresult != 0 ||
      other.last_loss->scope != Dx12DredScope::site_local ||
      other.last_loss->native_epoch != Dx12DredUnknown::unknown ||
      other.last_loss->breadcrumb_count != Dx12DredUnknown::unknown)
    return dred_fail(7, "QI acquisition must not invent exact identity or numeric breadcrumbs");
  return 0;
}
