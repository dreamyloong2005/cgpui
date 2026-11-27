#include "dx12_pacing_native_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include <limits>

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline{DesktopTestAccess::exclusive};
  Dx12PacingNativeFixture fixture;
  if (!deadline.done || !fixture.initialize() || fixture.api.sets != 1 || fixture.api.gets != 1)
    return dx12_fail(1, "pacing actual flagged-chain/debug fixture unavailable");
  auto abandoned = fixture.begin_admitted();
  if (!abandoned || fixture.api.ready != 1 || fixture.api.polls < 1 || fixture.state->submitted)
    return dx12_fail(2, "first CPU frame did not follow a genuine native admission");
  const auto polls = fixture.api.polls;
  const HANDLE observed = fixture.api.observed_handle;
  abandoned.reset(); // No GPU work/Present: retain the consumed native observation.
  if (!dx12_resize(*fixture.state, {96, 96}, {1}) || fixture.api.sets != 1 ||
      fixture.api.gets != 1 || fixture.api.closes || fixture.api.observed_handle != observed)
    return dx12_fail(3, "same-chain pre-first-Present resize reacquired or closed admission");
  auto invalid = fixture.begin_admitted();
  if (!invalid) return dx12_fail(10, "pre-Present error fixture failed admission");
  invalid->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  const auto rejected = invalid->present(); // Validity rejection precedes native submission/Present.
  if (rejected || rejected.error().code != ErrorCode::invalid_argument || fixture.state->submitted ||
      fixture.api.polls != polls)
    return dx12_fail(11, "pre-Present validation failure consumed or replaced native admission");
  invalid.reset();
  auto first = fixture.begin_admitted();
  if (!first || fixture.api.polls != polls || !fixture.present_pixels(*first, false))
    return dx12_fail(4, "abandoned first grant was lost, double-consumed, or pixels failed");
  const auto after_first_polls = fixture.api.polls;
  auto second = fixture.begin_admitted();
  if (!second || fixture.api.polls <= after_first_polls || fixture.api.ready != 2 ||
      !fixture.present_pixels(*second, true))
    return dx12_fail(5, "next present reused stale readiness or retained stale red pixels");
  auto resized_abandon = fixture.begin_admitted();
  if (!resized_abandon) return dx12_fail(6, "post-Present admission unavailable");
  resized_abandon.reset();
  const auto before_resize_polls = fixture.api.polls;
  if (!dx12_resize(*fixture.state, {64, 64}, {1}))
    return dx12_fail(7, "same-chain steady resize failed");
  auto resized = fixture.begin_admitted();
  if (!resized || fixture.api.polls != before_resize_polls ||
      !fixture.present_pixels(*resized, false) || fixture.api.gets != 1 || fixture.api.sets != 1)
    return dx12_fail(8, "same-chain resize changed grant/handle or failed real capture");
  fixture.pacing.reset();
  fixture.pacing.reset();
  if (fixture.api.closes != 1 || !fixture.api.close_ok || !fixture.debug_clean())
    return dx12_fail(9, "app-owned native latency handle not closed exactly once");
  std::cout << "PACING_NATIVE sets=" << fixture.api.sets << " gets=" << fixture.api.gets
            << " polls=" << fixture.api.polls << " ready=" << fixture.api.ready
            << " pending=" << fixture.api.pending << " closes=" << fixture.api.closes << '\n';
  return 0;
}
