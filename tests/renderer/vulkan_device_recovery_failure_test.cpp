#include "vulkan_device_recovery_scene_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(VulkanRebuildFailure failure) {
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoveryFailure", L"Vulkan reconstruction failure");
  if (!hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto image = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &image);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  const auto before = (*renderer)->backend_info();
  auto frame = recovery_scene_frame(**renderer);
  if (!frame) return 4;
  hooks.arm(VulkanLossSite::submit, VK_ERROR_DEVICE_LOST, failure);
  const auto failed = (*frame)->present();
  if (failed || recovery_retry(failed) || failed.error().code == ErrorCode::renderer_recovery_pending)
    return recovery_fail(5, "failed reconstruction pretended success/retry");
  const auto first = hooks.observation();
  const unsigned successful_creates = failure == VulkanRebuildFailure::device ? 1 : 2;
  if (!first.reliable || !first.injected || !first.safely_quiesced || first.device_attempts != 2 ||
      first.devices_created != successful_creates || first.devices_destroyed != successful_creates ||
      first.live_devices || first.rebuild_failures != 1 || first.instances_created != 1 ||
      first.instances_destroyed || first.surfaces_created != 1 || first.surfaces_destroyed ||
      (*renderer)->backend_info().device_generation != before.device_generation)
    return recovery_fail(6, "partial reconstruction was leaked/published or retained native device");
  for (unsigned retry = 0; retry < 3; ++retry) {
    const auto begin = (*renderer)->begin_frame();
    const auto present = (*frame)->present();
    const auto resize = (*renderer)->resize(window.framebuffer_size(), {1});
    if (begin || present || resize || begin.error().code != failed.error().code ||
        present.error().code != failed.error().code || resize.error().code != failed.error().code ||
        begin.error().message != failed.error().message || present.error().message != failed.error().message ||
        resize.error().message != failed.error().message || hooks.observation().device_attempts != 2)
      return recovery_fail(7, "terminal error was lost or failure restarted a reconstruction loop");
  }
  if ((*renderer)->last_frame_pixels() && !recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(8, "failed rebuild published a new capture");
  frame->reset(); renderer->reset();
  const auto retired = hooks.observation();
  return retired.reliable && !retired.live_devices && retired.instances_destroyed == 1 &&
      retired.surfaces_destroyed == 1 ? 0 : recovery_fail(9, "terminal teardown lost instance/surface ownership");
}
}
int main() {
  VulkanRecoveryDeadline deadline;
  if (!deadline.valid()) return 20;
  for (auto failure : {VulkanRebuildFailure::device, VulkanRebuildFailure::swapchain})
    if (const auto result = run(failure)) return result;
  return 0;
}
