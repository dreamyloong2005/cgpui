#include "dx12_composition_probe_test_support.hpp"
#include "../../src/renderer/dx12/dx12_composition_internal.hpp"
#include <chrono>
#include <cstdio>
#include <tuple>

namespace cgpui_test {
bool Dx12CompositionProbe::await_admission(bool before_commit) {
  using namespace cgpui;
  using Clock = std::chrono::steady_clock;
  if (!invariant(state && state->composition_host && windows.visibility_unchanged(),
      "admission fixture changed")) return false;
  if (before_commit && !invariant(!state->composition_host->committed && !state->submitted &&
      state->fence_value == 0, "first wait was primed by Commit/submission")) return false;
  const auto safety = [&] { return std::make_tuple(state->submitted, state->fence_value,
      state->pending_fence_value, state->failed, state->recovery_pending,
      state->info.device_generation, state->composition_host->committed); };
  const auto before = safety();
  const auto end = Clock::now() + std::chrono::seconds(3);
  do {
    const auto admitted = state->frame_pacing->try_admit();
    if (!admitted) {
      outcome = CompositionProbeExit::unmeasured;
      composition_probe_note("admission_error", admitted.error().message.c_str());
      return false;
    }
    if (!invariant(safety() == before && windows.visibility_unchanged(),
        "admission changed successful/native state")) return false;
    if (*admitted == Dx12FrameAdmission::ready) {
      std::printf("COMPOSITION_PROBE stage=admitted before_commit=%d ready=%u polls=%u pending=%u\n",
          before_commit, api.ready, api.polls, api.pending);
      return true;
    }
    if (!invariant(*admitted == Dx12FrameAdmission::pending, "flagged owner unexpectedly disabled"))
      return false;
    if (Clock::now() >= end) break;
    if (!composition_probe_dispatch()) return invariant(false, "WM_QUIT interrupted probe");
    if (Clock::now() >= end) break;
    Sleep(1);
  } while (Clock::now() < end);
  outcome = CompositionProbeExit::pending;
  composition_probe_note("bounded_pending", "no native readiness within 3s; NOT classified unsupported");
  return false;
}
} // namespace cgpui_test
