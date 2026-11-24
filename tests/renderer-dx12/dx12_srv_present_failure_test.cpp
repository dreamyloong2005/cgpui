#include "dx12_srv_pool_test_support.hpp"
#include "../../src/renderer/dx12/dx12_backdrop_blur_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Dx12Window window;
  SrvFailingHeapApi api; // Must outlive state's pool.
  auto state = srv_native_state(&window);
  if (!state) return dx12_fail(1, "SRV failed-present fixture unavailable");
  state->srv_pool = std::make_unique<Dx12SrvDescriptorPool>(8, &api);
  if (!srv_render_tiles(state, true, false)) return dx12_fail(2, "eight live slots did not produce exact pixels");
  const auto pixels = state->pixels->rgba8;
  const auto diagnostics = *state->diagnostics;
  std::vector<SrvNativeObservation> before, after;
  if (!srv_native_observations(*state, before)) return dx12_fail(3, "initial native topology missing");
  api.fail_on = 2;
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({1, 0, 1, 1});
  for (unsigned index = 0; index != 8; ++index)
    frame.draw_image({.bounds = {{float(index % 4 * 16), float(index / 4 * 16)}, {16, 16}},
        .asset = describe_image_asset(srv_tile_image(index))});
  frame.draw_backdrop_blur({.rect = {{0, 40}, {16, 16}}, .blur_radius = 1});
  if (!frame.request_pixel_capture()) return dx12_fail(4, "failed-frame capture request refused early");
  const auto result = frame.present();
  if (result || result.error().message.find("80070005") == std::string::npos ||
      !state->pixels || state->pixels->rgba8 != pixels || !diagnostic_same(&*state->diagnostics, diagnostics) ||
      state->submitted || !state->backdrop_blur->frame_targets.empty() ||
      state->srv_pool->stats().page_count != 1 || state->srv_pool->stats().live_slots != 8 ||
      api.attempts != 2 || api.successes != 1 || !srv_native_observations(*state, after) || before.size() != after.size())
    return dx12_fail(5, "allocation failure submitted/dropped draws or published partial capture/diagnostics");
  for (std::size_t i = 0; i != before.size(); ++i)
    if (before[i].heap_identity != after[i].heap_identity || before[i].gpu != after[i].gpu)
      return dx12_fail(6, "allocation failure mutated an old live native descriptor");
  return 0;
}
