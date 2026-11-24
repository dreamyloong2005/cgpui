#include "vulkan_device_recovery_scene_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(VulkanLossSite site) {
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoveryNonloss", L"Vulkan non-loss error preservation");
  if (!hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto image = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &image);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  const auto before = (*renderer)->backend_info();
  hooks.arm(site, VK_ERROR_OUT_OF_HOST_MEMORY);
  Result<void> failed;
  if (site == VulkanLossSite::resize_wait) {
    failed = (*renderer)->resize(window.framebuffer_size(), {1});
  } else {
    auto frame = recovery_scene_frame(**renderer);
    if (!frame) return 4;
    (*frame)->clear({0, 0, 1, 1});
    failed = (*frame)->present();
  }
  const auto o = hooks.observation();
  if (failed || recovery_retry(failed) || failed.error().code == ErrorCode::renderer_recovery_pending ||
      !o.reliable || !o.injected || !o.safely_quiesced || o.device_attempts != 1 || o.devices_created != 1 ||
      o.devices_destroyed || o.live_devices != 1 || (*renderer)->backend_info().device_generation != before.device_generation)
    return recovery_fail(5, "non-device-loss failure was swallowed or rebuilt native device");
  if (!recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(6, "non-loss failure destroyed/replaced the last successful capture");
  return 0;
}
}
int main() {
  VulkanRecoveryDeadline deadline;
  if (!deadline.valid()) return 20;
  for (auto site : {VulkanLossSite::submit, VulkanLossSite::present, VulkanLossSite::acquire,
      VulkanLossSite::pacing_wait, VulkanLossSite::capture_wait, VulkanLossSite::resize_wait})
    if (const auto result = run(site)) return result;
  return 0;
}
