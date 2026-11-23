#include "dx12_pacing_script_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12PacingScript script;
  {
    Dx12FramePacing pacing(script);
    const auto unset = pacing.try_admit();
    if (unset || unset.error().code != ErrorCode::invalid_argument)
      return pacing_fail(1, "unconfigured owner invented readiness");
    if (!pacing.configure(nullptr, false) ||
        !pacing_is(pacing.try_admit(), Dx12FrameAdmission::disabled) ||
        pacing.configure(nullptr, false) || script.sets || script.gets || script.polls)
      return pacing_fail(2, "disabled route touched native API or allowed reconfiguration");
    pacing.presentation_attempted();
    pacing.reset();
    if (!script.closed.empty()) return pacing_fail(3, "disabled route closed a nonexistent handle");
    // Only this scripted API accepts null chain. No real DXGI calls occur here.
    if (!pacing.configure(nullptr, true) || script.sets != 1 || script.gets != 1 ||
        script.requested_latency != 1)
      return pacing_fail(4, "configuration did not set/query exactly once");
    if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::pending) || script.polls != 1 ||
        !pacing_is(pacing.try_admit(), Dx12FrameAdmission::pending) || script.polls != 2)
      return pacing_fail(5, "timeout did not stay pending and reusable");
    script.waited = {WAIT_OBJECT_0, ERROR_ACCESS_DENIED};
    if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::ready) || script.polls != 3)
      return pacing_fail(6, "actual ready status was not accepted");
    script.waited = {WAIT_TIMEOUT, ERROR_SUCCESS};
    if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::ready) || script.polls != 3)
      return pacing_fail(7, "abandoning an unpresented admission consumed its grant");
    const auto duplicate = pacing.configure(nullptr, true);
    if (duplicate || duplicate.error().code != ErrorCode::invalid_argument ||
        script.sets != 1 || script.gets != 1 || !script.closed.empty() ||
        !pacing_is(pacing.try_admit(), Dx12FrameAdmission::ready))
      return pacing_fail(8, "duplicate configuration replaced the live handle/grant");
    pacing.presentation_attempted();
    if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::pending) || script.polls != 4)
      return pacing_fail(9, "Present attempt recycled the old grant");
    script.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
    if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::ready))
      return pacing_fail(10, "new admission unavailable");
    pacing.reset();
    pacing.reset();
    if (script.closed.size() != 1 || script.closed[0] != script.acquired || pacing.try_admit())
      return pacing_fail(11, "reset failed exact close or retained readiness");
    script.waited = {WAIT_TIMEOUT, ERROR_SUCCESS};
    // Numeric handle reuse is legitimate; the old chain's grant must still be gone.
    if (!pacing.configure(nullptr, true) ||
        !pacing_is(pacing.try_admit(), Dx12FrameAdmission::pending) ||
        script.sets != 2 || script.gets != 2)
      return pacing_fail(12, "recreated chain inherited a grant or missed acquisition");
  }
  return script.closed.size() == 2 ? 0 : pacing_fail(13, "destructor did not close remaining handle once");
}
