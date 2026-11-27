#include "dx12_pacing_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_internal.hpp"
#include "../../src/renderer/dx12/dx12_native_retirement_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  Dx12PacingNativeFixture fixture;
  if (!deadline.done || !fixture.initialize()) return dx12_fail(1, "pacing lifetime fixture unavailable");
  auto first = fixture.begin_admitted();
  if (!first || !fixture.present_pixels(*first, false)) return dx12_fail(2, "pacing lifetime pixels unavailable");
  first.reset();
  auto abandoned = fixture.begin_admitted();
  if (!abandoned) return dx12_fail(3, "pacing lifetime unconsumed native grant unavailable");
  abandoned.reset();
  const auto before = fixture.state->info;
  if (!dx12_remove_native_device(before)) return dx12_fail(4, "pacing actual RemoveDevice failed");
  // Stage A manual teardown/rebuild, not production recovery integration.
  // Removal is real proof; pacing itself supplies no fence-completion evidence.
  fixture.pacing.reset();
  dx12_release_native_objects(*fixture.state);
  if (fixture.api.closes != 1 || !fixture.api.close_ok || fixture.pacing.try_admit())
    return dx12_fail(5, "old latency handle/grant survived removed-device teardown");
  if (!dx12_create_device_on_adapter(*fixture.state, *before.adapter_luid) ||
      !dx12_create_pipeline(*fixture.state) || !fixture.create_chain() ||
      fixture.api.sets != 2 || fixture.api.gets != 2 ||
      fixture.state->info.adapter_luid != before.adapter_luid)
    return dx12_fail(6, "pacing owner pinned removed COM objects or failed native recreation");
  const auto polls = fixture.api.polls;
  auto replacement = fixture.begin_admitted();
  if (!replacement || fixture.api.polls <= polls || !fixture.present_pixels(*replacement, true) ||
      !dx12_recovered_native_device_clean(fixture.state->info))
    return dx12_fail(7, "new chain reused an old grant or failed fresh native pixels");
  fixture.pacing.reset();
  return fixture.api.closes == 2 && fixture.api.close_ok
      ? 0 : dx12_fail(8, "each native lifetime must close exactly one app-owned handle");
}
