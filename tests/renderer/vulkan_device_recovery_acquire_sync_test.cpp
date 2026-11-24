#include "vulkan_device_recovery_scene_test_support.hpp"
#include "vulkan_acquire_sync_failure_test_support.hpp"
#include <iostream>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanAcquireSync", L"Vulkan failed acquire sync repair");
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto image = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &image);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  const auto before = (*renderer)->backend_info();
  VulkanAcquireSyncFailureHook sync;
  if (!sync.installed()) return 4;
  auto failed_frame = recovery_scene_frame(**renderer);
  if (!failed_frame) return 5;
  hooks.arm(VulkanLossSite::acquire, VK_ERROR_OUT_OF_HOST_MEMORY);
  const auto failed = (*failed_frame)->present();
  const auto fault = sync.observation();
  if (failed || failed.error().code != ErrorCode::renderer_initialization_failed ||
      !fault.injected || fault.semaphore_attempts != 1 || !fault.semaphore_destroys || !fault.fence_destroys ||
      !fault.idle_successes || fault.teardown_before_idle || fault.create_before_idle ||
      fault.null_fence_waits || !recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(6, "failed acquire did not retire sync before its one-shot semaphore failure");
  failed_frame->reset();
  auto blocked_frame = recovery_scene_frame(**renderer);
  if (!blocked_frame) return 7;
  const auto blocked = (*blocked_frame)->present();
  const auto blocked_sync = sync.observation();
  if (blocked || blocked.error().code != ErrorCode::renderer_initialization_failed ||
      blocked_sync.fence_waits != fault.fence_waits || blocked_sync.null_fence_waits ||
      blocked_sync.semaphore_attempts != fault.semaphore_attempts)
    return recovery_fail(8, "partial sync allowed presentation or waited on a missing fence");
  blocked_frame->reset();
  if (!(*renderer)->resize(window.framebuffer_size(), {1}))
    return recovery_fail(9, "resize did not rebuild missing image semaphore and fence");
  const auto restored = recovery_scene_capture(**renderer);
  const auto native = hooks.observation();
  const auto repaired = sync.observation();
  if (!restored || !recovery_same_pixels(&*restored, *baseline) || repaired.null_fence_waits ||
      repaired.semaphore_attempts <= fault.semaphore_attempts ||
      !native.reliable || !native.injected || !native.safely_quiesced ||
      native.device_attempts != 1 || native.devices_created != 1 || native.devices_destroyed ||
      native.live_devices != 1 || (*renderer)->backend_info().device_generation != before.device_generation)
    return recovery_fail(10, "resize failed to restore capture without rebuilding the healthy device");
  std::cout << "VULKAN_ACQUIRE_SYNC simulated_oom=1 real_idle=success semaphore_oom=1"
            << " blocked_null_fence_waits=0 resize_restored=1 generation=" << before.device_generation << '\n';
  return 0;
}
