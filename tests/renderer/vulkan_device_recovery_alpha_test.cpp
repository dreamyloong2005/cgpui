#include "vulkan_device_recovery_scene_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoveryAlpha", L"Vulkan alpha recovery descriptor");
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto before = (*renderer)->backend_info();
  hooks.arm(VulkanLossSite::resize_wait);
  const auto lost = (*renderer)->set_transparent_surface(true);
  if (!lost || !lost->transparent_requested ||
      !recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation()) ||
      !(*renderer)->surface_composition().transparent_requested) return 3;
  // Recovery installed the desired surface. A repeated and inverse request must
  // not hit a false no-op caused by rolling back only the CPU descriptor.
  const auto retry = (*renderer)->set_transparent_surface(true);
  const auto opaque = (*renderer)->set_transparent_surface(false);
  if (!retry || !retry->transparent_requested || !opaque || opaque->transparent_requested ||
      (*renderer)->surface_composition().transparent_requested || hooks.observation().device_attempts != 2)
    return recovery_fail(4, "recovered alpha descriptor differs from installed surface");
  hooks.arm(VulkanLossSite::resize_wait, VK_ERROR_DEVICE_LOST, VulkanRebuildFailure::device);
  const auto terminal = (*renderer)->set_transparent_surface(true);
  if (terminal) return 5;
  const auto repeated = (*renderer)->set_transparent_surface(false);
  if (repeated || repeated.error().message != terminal.error().message || hooks.observation().device_attempts != 3)
    return recovery_fail(6, "terminal device allowed a false successful surface no-op");
  return 0;
}
