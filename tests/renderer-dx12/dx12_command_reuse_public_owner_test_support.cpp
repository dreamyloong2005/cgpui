#include "dx12_command_reuse_public_owner_test_support.hpp"
#include "../../src/renderer/dx12/dx12_device_cohort_internal.hpp"
#include "../../src/renderer/dx12/dx12_operation_internal.hpp"

namespace cgpui_test {
std::weak_ptr<cgpui::Dx12State> reuse_public_state(const cgpui::RendererBackendInfo& info, HWND hwnd) {
  cgpui::Dx12State lookup; // Empty, unregistered identity key; never a replacement renderer/state.
  lookup.info.adapter_luid = info.adapter_luid;
  lookup.window_thread_id = GetCurrentThreadId();
  std::weak_ptr<cgpui::Dx12State> result;
  for (const auto& peer : cgpui::dx12_device_cohort_peers(lookup)) {
    const auto state = peer.lock();
    if (!state) continue;
    cgpui::Dx12OperationGuard guard(*state);
    if (state->hwnd == hwnd && state->info.adapter_luid == info.adapter_luid &&
        state->info.device_generation == info.device_generation) {
      if (!result.expired()) return {}; // Ambiguous fixture must fail, never pick arbitrary state.
      result = peer;
    }
  }
  return result; // Weak only: no COM or state owner escapes fixture lookup.
}
void reuse_public_scene(cgpui::RenderFrame& frame, bool upload) {
  using namespace cgpui;
  const ImageAsset patterned{.id = {9901}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 8,
          .pixels = {0,255,0,255,0,0,255,255,255,255,255,255,0,0,0,255}}};
  frame.clear({0.02F,0.04F,0.07F,1});
  if (upload) { frame.upload_image(srv_tile_image(0)); frame.upload_image(patterned); }
  frame.draw_rect({.rect = {{1,1},{27,27}}, .color = {0.1F,0.1F,0.8F,1}});
  frame.draw_image({.bounds = {{4,4},{18,17}}, .asset = describe_image_asset(srv_tile_image(0)),
      .sampling = ImageSamplingMode::nearest});
  frame.draw_rounded_rect({.rect = {{10,9},{16,15}}, .color = {1,1,0,1}, .radius = BorderRadii::all(3)});
  frame.draw_image({.bounds = {{15,14},{13,13}}, .asset = describe_image_asset(patterned),
      .sampling = ImageSamplingMode::linear});
}
} // namespace cgpui_test
