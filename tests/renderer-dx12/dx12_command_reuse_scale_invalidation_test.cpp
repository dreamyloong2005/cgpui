#include "dx12_command_reuse_native_test_support.hpp"
#include <algorithm>

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline; Dx12Window window; auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "reuse scale native setup");
  state->command_observation = std::make_unique<Dx12CommandObservation>(16384);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true) || !reuse_scene(state, false, false)) return dx12_fail(2, "reuse scale fresh");
  const auto old = verifier->pixels.rgba8;
  const auto count = [&] { return std::ranges::count(state->command_observation->events(),
      Dx12CommandEventKind::bundle_created, &Dx12CommandEvent::kind); };
  if (count() != 1 || !dx12_resize(*state, {float(state->width), float(state->height)}, {2}))
    return dx12_fail(3, "reuse accepted scale-only resize");
  if (!reuse_scene(state, false, false) || count() != 2 || verifier->failed ||
      verifier->completed_parent != verifier->parent || verifier->pixels.rgba8 == old || !dx12_check_debug_errors(*state))
    return dx12_fail(4, "scale-only early return reused old native recording or stale tail pixels");
  const auto scaled = verifier->pixels.rgba8;
  // Independent public capture is a DIRECT control of the changed scene, not the hit-frame oracle.
  if (!reuse_scene(state, false, true) || state->pixels->rgba8 != scaled || count() != 2)
    return dx12_fail(5, "scaled same-frame tail disagrees with explicit DIRECT control");
  return 0;
}
