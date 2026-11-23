#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int blocked_generation(Renderer& renderer, const ImageAssetDescriptor& image) {
  const auto before = renderer.backend_info();
  auto held_device = dx12_acquire_native_device(before);
  if (!held_device) return dx12_fail(10, "external native device unavailable");
  if (!dx12_remove_native_device(before)) return dx12_fail(14, "blocked native removal failed");
  for (unsigned attempt = 0; attempt < 2; ++attempt) {
    const auto blocked = renderer.begin_frame();
    if (blocked || blocked.error().code != ErrorCode::renderer_initialization_failed)
      return dx12_fail(15, "blocked recovery accepted work or falsely requested frame retry");
    const auto info = renderer.backend_info();
    if (info.device_generation != before.device_generation || info.adapter_luid != before.adapter_luid ||
        renderer.last_frame_pixels()) return dx12_fail(16, "blocked recovery published a new generation");
  }
  held_device.Reset();
  // A manual later call may retry after the external obstacle is gone. Runtime
  // sees hard errors above and must not spin on an unusable renderer meanwhile.
  const auto retry = renderer.begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(17, "same renderer could not recover after blocker release");
  const auto after = renderer.backend_info();
  if (after.device_generation != before.device_generation + 1 || after.adapter_luid != before.adapter_luid)
    return dx12_fail(18, "blocked attempts consumed generations or changed adapter");
  auto frame = renderer.begin_frame();
  if (!frame) return dx12_fail(19, "unblocked renderer is not ready");
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = image});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel(renderer.last_frame_pixels(), 8, 8, {0, 255, 0, 255}) ||
      !dx12_pixel(renderer.last_frame_pixels(), 40, 40, {0, 0, 255, 255}))
    return dx12_fail(20, "CPU image cache was lost across blocked recovery");
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(21, "unblocked device debug errors");
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "blocker recovery fixture unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const ImageAsset image{.id = {8403}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {0, 255, 0, 255}}};
  auto warmup = (*renderer)->begin_frame();
  if (!warmup) return dx12_fail(3, "blocker warmup begin failed");
  (*warmup)->upload_image(image);
  (*warmup)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {0, 255, 0, 255}))
    return dx12_fail(4, "blocker warmup failed");
  return blocked_generation(**renderer, describe_image_asset(image));
}
