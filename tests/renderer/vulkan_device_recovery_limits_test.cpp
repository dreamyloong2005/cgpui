#include "vulkan_device_recovery_scene_test_support.hpp"
#include "vulkan_device_recovery_image_limits_test_support.hpp"
#include "../support/frame_wait_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
namespace {
Result<std::unique_ptr<RenderFrame>> image_frame(Renderer& renderer,
    const ImageAsset* image = nullptr, bool upload = false) {
  auto frame = wait_for_test_frame(renderer);
  if (!frame) return std::unexpected(frame.error());
  (*frame)->clear({0, 0, 0, 1});
  if (image) {
    if (upload) (*frame)->upload_image(*image);
    (*frame)->draw_image({.bounds = {{8, 8}, {24, 24}}, .asset = describe_image_asset(*image),
        .sampling = ImageSamplingMode::nearest});
  }
  if (auto capture = (*frame)->request_pixel_capture(); !capture)
    return std::unexpected(capture.error());
  return frame;
}
} // namespace
int main() {
  VulkanRecoveryDeadline deadline;
  VulkanRecoveryHooks hooks;
  RendererPixelTestWindow window(L"CGPUIVulkanRecoveryLimits", L"Vulkan retained image limit recovery");
  if (!deadline.valid() || !hooks.installed() || !window.valid()) return 1;
  auto renderer = create_vulkan_renderer({.native_surface = window.surface(),
      .framebuffer_size = window.framebuffer_size(), .scale = {1}});
  if (!renderer) return 2;
  ImageAsset image{.id = {2026112502}, .logical_size = {512, 1},
      .bitmap = {.width = 512, .height = 1, .stride = 2048, .pixels = std::vector<std::uint8_t>(2048)}};
  for (std::size_t i = 0; i < image.bitmap.pixels.size(); i += 4) {
    image.bitmap.pixels[i] = image.bitmap.pixels[i + 3] = 255;
  }
  auto seed = image_frame(**renderer, &image, true);
  if (!seed || !(*seed)->present() || !(*renderer)->last_frame_pixels() ||
      !pixel_near(*(*renderer)->last_frame_pixels(), 16, 16, {255, 0, 0, 255})) return 3;
  const auto baseline = *(*renderer)->last_frame_pixels();
  const auto before = (*renderer)->backend_info();
  seed->reset();
  auto loss = image_frame(**renderer);
  if (!loss) return 4;
  hooks.arm(VulkanLossSite::submit);
  if (!recovery_retry((*loss)->present()) ||
      !recovery_native_cycle(before, (*renderer)->backend_info(), hooks.observation())) return 5;
  loss->reset();
  const auto generation = (*renderer)->backend_info().device_generation;
  // Do not falsify capability during native creation/reconstruction. Only now
  // simulate a replacement device with smaller texture limits for restoration.
  VulkanImageLimitHook limits(hooks.observation().latest_physical, 8);
  if (!limits.installed()) return 6;
  auto empty = image_frame(**renderer); // No image draw/upload: retained-only path.
  if (!empty) return 7;
  const auto rejected = (*empty)->present();
  if (rejected || rejected.error().code != ErrorCode::renderer_initialization_failed ||
      limits.property_overrides() == 0 || limits.image_attempts() != 0 || limits.invalid_image_attempts() != 0 ||
      (*renderer)->backend_info().device_generation != generation || hooks.observation().device_attempts != 2 ||
      !recovery_same_pixels((*renderer)->last_frame_pixels(), baseline))
    return recovery_fail(8, "retained oversize reached native create or corrupted the last successful frame");
  empty->reset();
  limits.restore_properties();
  auto restored = image_frame(**renderer);
  if (!restored || !(*restored)->present() || limits.image_attempts() == 0 || limits.invalid_image_attempts() != 0)
    return recovery_fail(9, "restoration did not resume after the capability mismatch was removed");
  restored->reset();
  auto cached = image_frame(**renderer, &image); // Never resupply the retained bytes.
  if (!cached || !(*cached)->present() || !(*renderer)->last_frame_pixels() ||
      !pixel_near(*(*renderer)->last_frame_pixels(), 16, 16, {255, 0, 0, 255}) ||
      (*renderer)->backend_info().device_generation != generation || hooks.observation().device_attempts != 2)
    return recovery_fail(10, "retained pixels were discarded or capability error restarted recovery");
  return 0;
}
