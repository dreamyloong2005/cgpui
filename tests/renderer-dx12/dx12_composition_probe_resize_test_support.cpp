#include "dx12_composition_probe_test_support.hpp"
#include <cstdio>

namespace cgpui_test {
bool Dx12CompositionProbe::resize_same_chain(UINT extent) {
  using namespace cgpui;
  if (!invariant(state && extent >= 64 && extent != state->width && !state->frame_active,
      "resize must really change a valid, inactive extent")) return false;
  const auto chain = state->swapchain.Get();
  const auto observed = api.observed_handle; // Non-owning identity within this same-chain operation only.
  const auto sets = api.sets, gets = api.gets, closes = api.closes;
  if (!result("resize_retirement", dx12_wait_idle(*state))) return false;
  state->pixels.reset();
  state->readback.Reset();
  for (auto& target : state->targets) target.Reset();
  // Keep this native experiment independent of production resize policy.
  if (!native("ResizeBuffers_same_creation_flags", state->swapchain->ResizeBuffers(
      2, extent, extent, DXGI_FORMAT_R8G8B8A8_UNORM, creation_flags_))) return false;
  state->width = state->height = extent;
  DXGI_SWAP_CHAIN_DESC1 actual{};
  if (!native("GetDesc1_after_resize", state->swapchain->GetDesc1(&actual)) ||
      !result("resize_targets", dx12_create_targets(*state))) return false;
  std::printf("COMPOSITION_PROBE resize extent=%u flags=%u\n", extent, actual.Flags);
  return invariant(windows.resize(extent) && actual.Width == extent && actual.Height == extent &&
      actual.Flags == creation_flags_ && state->swapchain.Get() == chain &&
      api.observed_handle == observed && api.sets == sets && api.gets == gets && api.closes == closes,
      "same-chain resize changed flags/handle/configuration/visibility") && debug_clean();
}
} // namespace cgpui_test
