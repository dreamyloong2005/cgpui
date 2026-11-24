#include "vulkan_device_recovery_scene_test_support.hpp"
#include "vulkan_image_create_failure_test_support.hpp"
#include "vulkan_image_texture_cache_internal.hpp"
#include "../support/frame_wait_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
Result<std::unique_ptr<RenderFrame>> empty_frame(Renderer& renderer, const ImageAsset* upload = nullptr) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return std::unexpected(frame.error());
  (*frame)->clear({0, 0, 0, 1});
  if (upload) (*frame)->upload_image(*upload);
  return frame;
}
} // namespace
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanEvictionFailure", L"Vulkan eviction failure retention");
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  const auto image = recovery_image();
  const auto baseline = recovery_scene_capture(**renderer, &image);
  if (!baseline || !recovery_scene_pixels(*baseline)) return 3;
  // Exactly age to the inclusive retention boundary; the next preparation must
  // evict the old native texture, then fail creating an unrelated replacement.
  for (std::uint64_t i = 0; i < vulkan_image_texture_cache_max_idle_frames; ++i) {
    auto idle = empty_frame(**renderer);
    if (!idle || !(*idle)->present()) return recovery_fail(4, "empty aging frame failed");
  }
  const auto* aged = (*renderer)->last_frame_diagnostic_snapshot();
  if (!aged || aged->retained_image_byte_count != std::optional<std::size_t>{4})
    return recovery_fail(5, "old image was not resident immediately before the eviction boundary");
  auto replacement = recovery_image({0, 255, 0, 255});
  replacement.id = {2026112503};
  VulkanImageCreateFailureHook fault;
  if (!fault.installed()) return 6;
  auto failed_frame = empty_frame(**renderer, &replacement);
  if (!failed_frame) return 7;
  const auto failed = (*failed_frame)->present();
  const auto o = fault.observation();
  if (failed || failed.error().code != ErrorCode::renderer_initialization_failed ||
      !o.injected || !o.safely_quiesced || o.create_attempts != 1 || !o.images_destroyed_before_failure ||
      !recovery_same_pixels((*renderer)->last_frame_pixels(), *baseline))
    return recovery_fail(8, "eviction/native CreateImage OOM did not occur in the required failed frame");
  failed_frame->reset();
  const auto before = (*renderer)->backend_info();
  // Lose the device without any intervening successful image preparation/prune.
  hooks.arm(VulkanLossSite::resize_wait);
  if (!(*renderer)->resize(window.framebuffer_size(), {1}) ||
      !recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation())) return 9;
  const auto absent = recovery_scene_capture(**renderer); // Never upload the old bytes again yet.
  const auto* sample = (*renderer)->last_frame_diagnostic_snapshot();
  if (!absent || !recovery_scene_pixels(*absent, {0, 0, 0, 255}) ||
      !sample || sample->retained_image_byte_count != std::optional<std::size_t>{0})
    return recovery_fail(10, "evicted or failed-upload CPU payload resurrected after native recovery");
  const auto restored = recovery_scene_capture(**renderer, &image);
  if (!restored || !recovery_scene_pixels(*restored) || hooks.observation().device_attempts != 2)
    return recovery_fail(11, "explicit reupload did not restore the evicted image on the recovered device");
  return 0;
}
