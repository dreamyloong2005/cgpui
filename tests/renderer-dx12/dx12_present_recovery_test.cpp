#include "dx12_recovery_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "present recovery fixture unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const ImageAsset image{.id = {8402}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto warmup = (*renderer)->begin_frame();
  if (!warmup) return dx12_fail(3, "present recovery warmup begin failed");
  (*warmup)->upload_image(image);
  (*warmup)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}))
    return dx12_fail(4, "present recovery warmup failed");
  const auto* borrowed = (*renderer)->last_frame_diagnostic_snapshot();
  if (!borrowed) return dx12_fail(14, "present recovery diagnostic warmup absent");
  const auto saved_diagnostics = *borrowed;
  const auto before = (*renderer)->backend_info();
  auto interrupted = (*renderer)->begin_frame();
  if (!interrupted) return dx12_fail(5, "interrupted frame begin failed");
  (*interrupted)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*interrupted)->request_pixel_capture() || !dx12_remove_native_device(before))
    return dx12_fail(6, "active-frame native removal failed");
  const auto result = (*interrupted)->present();
  if (result || result.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(7, "active-frame loss did not recover before present retry");
  if ((*renderer)->last_frame_diagnostic_snapshot() != borrowed ||
      !diagnostic_same(borrowed, saved_diagnostics))
    return dx12_fail(15, "failed present/reconstruction changed the last successful snapshot");
  const auto after = (*renderer)->backend_info();
  if (after.adapter_luid != before.adapter_luid ||
      after.device_generation != before.device_generation + 1 || (*renderer)->last_frame_pixels())
    return dx12_fail(8, "present recovery did not publish a clean next generation");
  const auto repeated = (*interrupted)->present();
  if (repeated || repeated.error().code != ErrorCode::invalid_argument)
    return dx12_fail(9, "interrupted frame was accepted twice");
  for (unsigned pass = 0; pass < 2; ++pass) {
    auto frame = (*renderer)->begin_frame();
    if (!frame) return dx12_fail(10, "present-recovered renderer cannot begin");
    if (pass == 1) {
      // Destruction of pre-loss frames must not release this new active frame.
      warmup->reset();
      interrupted->reset();
      if ((*renderer)->begin_frame()) return dx12_fail(11, "old frame destruction unlocked new frame");
    }
    (*frame)->clear({0, 0, 1, 1});
    (*frame)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 0, 255, 255}))
      return dx12_fail(12, "cached pixels failed after present recovery or old frame destruction");
    const auto* diagnostics = (*renderer)->last_frame_diagnostic_snapshot();
    if (!diagnostics || diagnostics->upload_bytes.image_byte_count != (pass ? 0U : 4U))
      return dx12_fail(16, "present recovery image upload counted cached or missed fresh bytes");
  }
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(13, "recovered device debug errors");
}
