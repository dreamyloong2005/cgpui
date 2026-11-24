#include "dx12_srv_pool_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  auto state = srv_native_state(&window);
  if (!state || !srv_render_tiles(state, true, true) || !srv_topology(*state, 14, 1))
    return dx12_fail(1, "cold shared image/blur frame failed");
  std::vector<SrvNativeObservation> cold;
  if (!srv_native_observations(*state, cold) || cold.empty() ||
      state->srv_pool->stats().successful_page_creations != 1) return dx12_fail(2, "cold native observations unavailable");
  const auto identity = cold.front().heap_identity;
  for (unsigned pass = 0; pass != 3; ++pass) {
    if (!srv_render_tiles(state, false, true) || !srv_topology(*state, 14, 1) ||
        state->srv_pool->stats().successful_page_creations != 1 || state->srv_pool->stats().live_slots != 14)
      return dx12_fail(3, "warm image/blur frame allocated extra heap or changed pixels");
    std::vector<SrvNativeObservation> warm;
    if (!srv_native_observations(*state, warm) || warm.empty() || warm.front().heap_identity != identity)
      return dx12_fail(4, "warm frame replaced its stable native page");
  }
  return dx12_check_debug_errors(*state) ? 0 : dx12_fail(5, "shared heap GPU validation error");
}
