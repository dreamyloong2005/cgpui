#include "vulkan_device_recovery_scene_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
int run(VulkanLossSite site) {
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoverySites", L"Vulkan simulated loss sites");
  if (!hooks.installed() || !window.valid()) return recovery_fail(1, "site fixture unavailable");
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return recovery_fail(2, "native Vulkan factory failed");
  const auto image = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &image);
  if (!baseline || !recovery_scene_pixels(*baseline)) return recovery_fail(3, "baseline image/glyph absent");
  const auto before = (*renderer)->backend_info();
  hooks.arm(site);
  if (site == VulkanLossSite::resize_wait) {
    // Same actual client extent isolates device recovery from HWND resize races.
    if (!(*renderer)->resize(window.framebuffer_size(), {1}))
      return recovery_fail(4, "resize did not finish requested state after loss");
  } else {
    auto frame = recovery_scene_frame(**renderer);
    if (!frame) return recovery_fail(5, "loss frame unavailable");
    (*frame)->clear({0, 0, 1, 1}); // A failed candidate must never replace baseline history.
    if (!recovery_retry((*frame)->present())) return recovery_fail(6, "loss Present was not a typed retry");
  }
  if (!recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation()))
    return recovery_fail(7, "native rebuild/identity/lifetime evidence differs");
  if ((*renderer)->last_frame_pixels() && !recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(8, "failed capture candidate published instead of prior history/reset");
  const auto recovered = recovery_scene_capture(**renderer); // No image upload after loss.
  if (!recovered || !recovery_same_pixels(&*recovered, *baseline) || !recovery_scene_pixels(*recovered))
    return recovery_fail(9, "retained image/glyph pixels not restored exactly");
  const auto generation = (*renderer)->backend_info().device_generation;
  auto uncaptured = recovery_scene_frame(**renderer, nullptr, false);
  if (!uncaptured || !(*uncaptured)->present() || !recovery_same_pixels((*renderer)->last_frame_pixels(), *recovered) ||
      (*renderer)->backend_info().device_generation != generation)
    return recovery_fail(10, "warm uncaptured frame changed history/generation");
  uncaptured->reset(); renderer->reset();
  const auto retired = hooks.observation();
  if (!retired.reliable || retired.live_devices || retired.devices_destroyed != 2 ||
      retired.instances_destroyed != 1 || retired.surfaces_destroyed != 1)
    return recovery_fail(11, "final native owners not retired once");
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
