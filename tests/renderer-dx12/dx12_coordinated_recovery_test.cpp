#include "../support/frame_wait_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
ImageAsset image(std::array<std::uint8_t, 4> color) {
  return {.id = {8501}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4,
          .pixels = {color[0], color[1], color[2], color[3]}}};
}
void paint(RenderFrame& frame, const ImageAsset& asset) {
  frame.clear({0, 0, 1, 1});
  frame.draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(asset)});
}
bool finish(Renderer& renderer, RenderFrame& frame, std::array<std::uint8_t, 4> color) {
  return frame.request_pixel_capture() && frame.present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, color) &&
      dx12_pixel(renderer.last_frame_pixels(), 40, 40, {0, 0, 255, 255});
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window first_window, second_window;
  if (!deadline.done || !first_window.hwnd || !second_window.hwnd)
    return dx12_fail(1, "coordinated recovery fixture unavailable");
  auto first = create_dx12_renderer(first_window.descriptor());
  auto second = create_dx12_renderer(second_window.descriptor());
  if (!first || !second) return dx12_fail(2, "coordinated renderers unavailable");
  const auto first_before = (*first)->backend_info(), second_before = (*second)->backend_info();
  if (!first_before.adapter_luid || first_before.adapter_luid != second_before.adapter_luid)
    return dx12_fail(3, "coordinated fixture did not select the same adapter");
  const auto red = image({255, 0, 0, 255}), green = image({0, 255, 0, 255});
  auto first_finished = cgpui_test::wait_for_test_frame(**first);
  auto second_finished = cgpui_test::wait_for_test_frame(**second);
  if (!first_finished || !second_finished) return dx12_fail(4, "coordinated warmup begin failed");
  (*first_finished)->upload_image(red);
  (*second_finished)->upload_image(green); // Same public ID, distinct per-renderer CPU caches.
  paint(**first_finished, red);
  paint(**second_finished, green);
  if (!finish(**first, **first_finished, {255, 0, 0, 255}) ||
      !finish(**second, **second_finished, {0, 255, 0, 255}))
    return dx12_fail(5, "coordinated warmup pixels differ");
  auto pending_second = cgpui_test::wait_for_test_frame(**second);
  if (!pending_second) return dx12_fail(6, "peer CPU frame begin failed");
  paint(**pending_second, green);
  if (!(*pending_second)->request_pixel_capture()) return dx12_fail(7, "peer capture request failed");
  const auto* borrowed_second = (*second)->last_frame_pixels();
  if (!borrowed_second) return dx12_fail(7, "peer warmup capture unavailable");
  const auto saved_second = borrowed_second->rgba8;
  if (!dx12_remove_native_device(first_before))
    return dx12_fail(7, "coordinated native removal failed");
  const auto first_retry = (*first)->begin_frame();
  if (first_retry || first_retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(8, "same-thread peer still blocks coordinated recovery");
  const auto first_after = (*first)->backend_info(), pending_info = (*second)->backend_info();
  if (first_after.device_generation != first_before.device_generation + 1 ||
      first_after.adapter_luid != first_before.adapter_luid ||
      pending_info.device_generation != second_before.device_generation ||
      pending_info.adapter_luid != second_before.adapter_luid)
    return dx12_fail(9, "cohort recovery did not publish generations lazily per renderer");
  if ((*first)->last_frame_pixels() || (*second)->last_frame_pixels())
    return dx12_fail(10, "removed-generation captures remain observable");
  // Peer purge must not invalidate a previously borrowed CPU capture. Only the
  // next operation on that peer may replace/reset its buffer; do not read later.
  if (borrowed_second->rgba8 != saved_second ||
      !dx12_pixel(borrowed_second, 8, 8, {0, 255, 0, 255}))
    return dx12_fail(11, "peer recovery invalidated the borrowed capture buffer");
  (*pending_second)->draw_rect({.rect = {{0, 0}, {16, 16}}, .color = {1, 0, 0, 1}});
  (*pending_second)->draw_backdrop_blur({.rect = {{4, 4}, {32, 32}}, .blur_radius = 4});
  const auto peer_retry = (*pending_second)->present();
  if (peer_retry || peer_retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(12, "pending peer frame did not rebuild before requesting retry");
  const auto second_after = (*second)->backend_info();
  if (second_after.device_generation != second_before.device_generation + 1 ||
      second_after.adapter_luid != second_before.adapter_luid || (*second)->last_frame_pixels())
    return dx12_fail(13, "peer rebuild generation or lost-frame capture differs");
  const auto repeated = (*pending_second)->present();
  if (repeated || repeated.error().code != ErrorCode::invalid_argument)
    return dx12_fail(14, "lost peer frame was presentable twice");
  auto first_new = cgpui_test::wait_for_test_frame(**first), second_new = cgpui_test::wait_for_test_frame(**second);
  if (!first_new || !second_new) return dx12_fail(15, "recovered cohort is not ready");
  first_finished->reset();
  second_finished->reset();
  pending_second->reset();
  const auto overlap = (*second)->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument)
    return dx12_fail(16, "old peer frame destruction unlocked the new frame");
  paint(**first_new, red); // No upload after loss on either renderer.
  paint(**second_new, green);
  if (!finish(**first, **first_new, {255, 0, 0, 255}) ||
      !finish(**second, **second_new, {0, 255, 0, 255}))
    return dx12_fail(17, "coordinated recovery lost or mixed cached images");
  return dx12_recovered_native_device_clean(first_after)
      ? 0 : dx12_fail(18, "coordinated recovered device debug errors");
}
