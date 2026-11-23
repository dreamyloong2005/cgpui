#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "resize recovery fixture unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  auto warmup = (*renderer)->begin_frame();
  if (!warmup) return dx12_fail(3, "resize recovery warmup begin failed");
  (*warmup)->clear({0, 1, 0, 1});
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present())
    return dx12_fail(4, "resize recovery warmup failed");
  const auto before = (*renderer)->backend_info();
  if (!dx12_remove_native_device(before)) return dx12_fail(5, "resize native removal failed");
  // Runtime resize does not consume frame_retry_required: success must mean the
  // device is rebuilt AND the requested physical extent/DPI has been applied.
  if (const auto resized = (*renderer)->resize({96, 48}, {2}); !resized)
    return dx12_fail(6, resized.error().message.c_str());
  const auto after = (*renderer)->backend_info();
  if (after.adapter_luid != before.adapter_luid ||
      after.device_generation != before.device_generation + 1 || (*renderer)->last_frame_pixels())
    return dx12_fail(7, "resize recovery generation or capture differs");
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(8, "resize-recovered renderer did not become ready");
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 1}});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present())
    return dx12_fail(9, "resized recovered frame failed");
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || pixels->width != 96 || pixels->height != 48 ||
      !dx12_pixel(pixels, 12, 12, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 20, 20, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 4, 4, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 28, 12, {0, 0, 255, 255}) ||
      !dx12_pixel(pixels, 95, 47, {0, 0, 255, 255}))
    return dx12_fail(10, "resize recovery ignored requested size or scale");
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(11, "resized device debug errors");
}
