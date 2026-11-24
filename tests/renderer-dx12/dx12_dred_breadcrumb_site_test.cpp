#include "dx12_dred_scalar_test_support.hpp"
using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12DredSite site; site.configure_setup(Dx12DredSetupRequestReturned{{0, true}}); site.open_local_lease();
  DredStatusScript api; api.reason = 0; api.query = {0, true};
  api.breadcrumbs = {.status = Dx12DredBreadcrumbStatus::sampled, .output_requested = true, .nodes = 2, .operations = 17};
  if (site.observe(api) || api.queries) return 1;
  api.reason = dred_removed;
  if (!site.observe(api) || api.queries != 1) return 2;
  const auto snapshot = site.snapshot_locked();
  if (!snapshot.last_loss || snapshot.last_loss->output != Dx12DredOutput::best_effort_non_atomic ||
      snapshot.last_loss->breadcrumbs.nodes != 2 || snapshot.last_loss->breadcrumbs.operations != 17 ||
      site.observe(api) || api.queries != 1) return 3;
  site.close_local_lease(); api.breadcrumbs.nodes = 99;
  if (snapshot.last_loss->breadcrumbs.nodes != 2 || site.snapshot_locked().last_loss->breadcrumbs.nodes != 2) return 4;
  Dx12DredSite off; off.open_local_lease(); api.queries = 0;
  if (!off.observe(api) || api.queries || off.snapshot_locked().last_loss->breadcrumbs.output_requested) return 5;
  return 0;
}
