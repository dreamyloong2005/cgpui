#include "dx12_command_reuse_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"
#include <algorithm>

namespace cgpui_test {
namespace {
cgpui::ImageAsset sampling_image() {
  return {.id = {9901}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 8,
          .pixels = {0, 255, 0, 255, 0, 0, 255, 255,
                     255, 255, 255, 255, 0, 0, 0, 255}}};
}
}
bool reuse_scene(const std::shared_ptr<cgpui::Dx12State>& state, bool upload, bool capture) {
  using namespace cgpui;
  state->frame_active = true;
  Dx12Frame frame(state);
  frame.clear({0.02F, 0.04F, 0.07F, 1});
  if (upload) { frame.upload_image(srv_tile_image(0)); frame.upload_image(sampling_image()); }
  frame.draw_rect({.rect = {{1, 1}, {27, 27}}, .color = {0.1F, 0.1F, 0.8F, 1}});
  frame.draw_image({.bounds = {{4, 4}, {18, 17}}, .asset = describe_image_asset(srv_tile_image(0)),
      .sampling = ImageSamplingMode::nearest});
  frame.draw_rounded_rect({.rect = {{10, 9}, {16, 15}}, .color = {1, 1, 0, 1}, .radius = BorderRadii::all(3)});
  frame.draw_image({.bounds = {{15, 14}, {13, 13}}, .asset = describe_image_asset(sampling_image()),
      .sampling = ImageSamplingMode::linear});
  if (capture && !frame.request_pixel_capture()) return false;
  return bool(frame.present());
}
bool reuse_trace_pixels_ready(const cgpui::Dx12State& state, const ReuseQueueTail& tail,
                             std::span<const std::uint8_t> expected) {
  using K = cgpui::Dx12CommandEventKind;
  void* target = nullptr; void* queue = nullptr;
  unsigned producer = 0, verifier = 0, completion = 0;
  for (const auto& event : state.command_observation->events()) if (event.parent == tail.parent) {
    if (event.kind == K::parent_target) { if (!event.a) return false; target = event.object; }
    if (event.kind == K::producer_execute) {
      if (!target || target != tail.target_identity || event.object2 != target) return false;
      queue = event.object; ++producer;
    }
    if (event.kind == K::tail_execute) {
      if (producer != 1 || !queue || event.object2 != queue || completion) return false;
      ++verifier;
    }
    if (event.kind == K::completed) {
      if (producer != 1 || verifier != 1 || event.a != tail.ticket) return false;
      ++completion;
    }
  }
  return !tail.failed && tail.parent && tail.completed_parent == tail.parent && tail.ticket &&
      producer == 1 && verifier == 1 && completion == 1 &&
      tail.target_identity && tail.pixels.rgba8.size() == expected.size() &&
      std::equal(expected.begin(), expected.end(), tail.pixels.rgba8.begin()) &&
      !state.submitted && state.info.backend == cgpui::RendererBackend::dx12 &&
      state.info.device_generation && state.info.adapter_luid && state.diagnostics &&
      state.diagnostics->submitted_draws.image_count == 2;
}
} // namespace cgpui_test
