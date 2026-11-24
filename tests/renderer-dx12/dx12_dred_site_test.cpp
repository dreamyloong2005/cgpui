#include "dx12_dred_scalar_test_support.hpp"
#include <type_traits>

using namespace cgpui;
using namespace cgpui_test;
static_assert(std::is_nothrow_copy_constructible_v<Dx12DredSiteSnapshot>);
static_assert(std::is_nothrow_copy_assignable_v<Dx12DredStatus>);
static_assert(sizeof(Dx12DredSite) <= 256);
int main() {
  Dx12DredSite site;
  DredStatusScript actual;
  actual.reason = dred_removed;
  if (site.observe(actual) || actual.reasons || site.snapshot_locked().last_loss)
    return dred_fail(1, "closed/no-device lease must not query or publish");
  site.configure_setup(Dx12DredNotRequested{});
  site.open_local_lease();
  actual.reason = 0;
  if (site.observe(actual) || site.observe(actual) || actual.reasons != 2 || actual.queries ||
      site.snapshot_locked().phase != Dx12DredSitePhase::open || site.snapshot_locked().last_loss)
    return dred_fail(2, "actual live reason cannot consume the later loss opportunity");
  actual.reason = dred_removed;
  if (!site.observe(actual)) return dred_fail(3, "actual failed reason was not published");
  const auto saved = site.snapshot_locked();
  if (saved.phase != Dx12DredSitePhase::complete || !saved.last_is_current || !saved.last_loss ||
      saved.last_loss->actual_removed_reason != dred_removed || actual.reasons != 3 || actual.queries ||
      !std::holds_alternative<Dx12DredNotRequested>(saved.last_loss->setup) ||
      !std::holds_alternative<Dx12DredDebugNotRequested>(saved.last_loss->interface_query))
    return dred_fail(4, "debug-false actual loss needs typed QI skip, not a fabricated HRESULT");
  actual.reason = dred_reset;
  if (site.observe(actual) || actual.reasons != 3 || actual.queries ||
      site.snapshot_locked().last_loss->actual_removed_reason != dred_removed)
    return dred_fail(5, "complete lease must not query or replace its immutable observation");
  site.close_local_lease();
  const auto closed = site.snapshot_locked();
  if (closed.phase != Dx12DredSitePhase::closed || closed.last_is_current || !closed.last_loss ||
      site.observe(actual) || actual.reasons != 3 || !saved.last_is_current)
    return dred_fail(6, "close must retain historical value without changing an earlier value copy");
  return 0;
}
