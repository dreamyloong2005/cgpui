#include "dx12_composition_probe_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include <cstdio>

namespace cgpui_test {
int run_composition_probe(bool visible) {
  Dx12RecoveryDeadline deadline{visible ? DesktopTestAccess::exclusive : DesktopTestAccess::none};
  if (!deadline.done) {
    composition_probe_note("watchdog", "unavailable; no capability observation");
    return static_cast<int>(CompositionProbeExit::unmeasured);
  }
  Dx12CompositionProbe probe;
  const auto exercise = [&]() -> bool {
    if (!probe.initialize(visible)) return false;
    if (visible && !probe.invariant(probe.windows.expect_backdrop(), "green desktop control unavailable"))
      return false;
    // Exact production bootstrap order: bound graph, no Commit/Present, native poll.
    if (!probe.await_admission(true) ||
        !probe.invariant(probe.api.ready == 1 && probe.api.polls >= 1, "first readiness not observed"))
      return false;
    const auto first_polls = probe.api.polls;
    // Retain that native grant across a real same-chain resize before first Present.
    if (!probe.resize_same_chain(96) || !probe.present_pixels(false) ||
        !probe.invariant(probe.api.polls == first_polls, "same-chain resize lost retained admission"))
      return false;
    const auto second_polls = probe.api.polls;
    if (!probe.present_pixels(true) ||
        !probe.invariant(probe.api.ready == 2 && probe.api.polls > second_polls,
                         "second Present reused old readiness")) return false;
    // Now resize a consumed-grant chain. A fresh native observation is required.
    if (!probe.resize_same_chain(64)) return false;
    const auto third_polls = probe.api.polls;
    if (!probe.present_pixels(false) ||
        !probe.invariant(probe.api.ready == 3 && probe.api.polls > third_polls,
                         "post-resize Present reused old readiness")) return false;
    return probe.debug_clean();
  };
  const bool observed = exercise();
  const bool closed = probe.close_and_verify(); // Also account for partial initialization failures.
  if (observed && closed) probe.outcome = CompositionProbeExit::observed;
  std::printf("COMPOSITION_PROBE result=%d mode=%s scope=native-experiment-only\n",
      static_cast<int>(probe.outcome), visible ? "visible" : "initially-hidden");
  return static_cast<int>(probe.outcome);
}
} // namespace cgpui_test
