#include "vulkan_device_recovery_scene_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoveryContent", L"Vulkan retained content recovery");
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto red = recovery_image(), green = recovery_image({0, 255, 0, 255});
  const auto baseline = recovery_scene_capture(**renderer, &red);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  const auto initial_generation = (*renderer)->backend_info().device_generation;
  auto finished = recovery_scene_frame(**renderer);
  if (!finished || !(*finished)->present()) return 15;
  const auto repeated = (*finished)->present();
  const auto repeated_capture = (*finished)->request_pixel_capture();
  if (repeated || repeated_capture || repeated.error().code != ErrorCode::invalid_argument ||
      repeated_capture.error().code != ErrorCode::invalid_argument ||
      (*renderer)->backend_info().device_generation != initial_generation ||
      !recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(16, "healthy finished frame accepted duplicate Present/capture");
  finished->reset();
  // Several legal CPU frames can exist. The old epoch must not replay its upload
  // or invalidation once another frame has reconstructed the native device.
  auto stale = recovery_scene_frame(**renderer, &red);
  if (!stale) return 4;
  (*stale)->invalidate_image(red.id);
  auto loss = recovery_scene_frame(**renderer, &green);
  if (!loss) return 5;
  hooks.arm(VulkanLossSite::submit);
  if (!recovery_retry((*loss)->present())) return recovery_fail(6, "submit did not recover");
  const auto generation = (*renderer)->backend_info().device_generation;
  if (!recovery_retry((*stale)->present()) || !recovery_retry((*loss)->present()) ||
      (*renderer)->backend_info().device_generation != generation)
    return recovery_fail(7, "old epoch frame replayed or repeated recovery");
  stale->reset(); loss->reset();
  const auto restored = recovery_scene_capture(**renderer);
  if (!restored || !recovery_scene_pixels(*restored, {0, 255, 0, 255}))
    return recovery_fail(8, "last accepted prepare upload did not survive loss");
  auto invalidation = recovery_scene_frame(**renderer);
  if (!invalidation) return 9;
  (*invalidation)->invalidate_image(red.id);
  if (!(*invalidation)->present() || !(*renderer)->last_frame_pixels() ||
      !recovery_scene_pixels(*(*renderer)->last_frame_pixels(), {0, 0, 0, 255}))
    return recovery_fail(10, "invalidation did not remove GPU/CPU image");
  invalidation->reset();
  auto second_loss = recovery_scene_frame(**renderer);
  if (!second_loss) return 11;
  hooks.arm(VulkanLossSite::present);
  if (!recovery_retry((*second_loss)->present())) return 12;
  second_loss->reset();
  const auto absent = recovery_scene_capture(**renderer);
  if (!absent || !recovery_scene_pixels(*absent, {0, 0, 0, 255}) ||
      (*renderer)->backend_info().device_generation != generation + 1)
    return recovery_fail(13, "invalidated CPU bytes resurrected after another loss");
  const auto* sample = (*renderer)->last_frame_diagnostic_snapshot();
  if (!sample || sample->retained_image_byte_count != std::optional<std::size_t>{0})
    return recovery_fail(14, "retained byte census kept invalidated payload");
  return 0;
}
