#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool captured_clear(Renderer& renderer, Color color, std::array<std::uint8_t, 4> expected) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear(color);
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 8, 8, expected);
}
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window first_window, begin_window, resize_window;
  if (!deadline.done || !first_window.hwnd || !begin_window.hwnd || !resize_window.hwnd)
    return dx12_fail(1, "lazy cohort fixture unavailable");
  auto first = create_dx12_renderer(first_window.descriptor());
  auto begin_peer = create_dx12_renderer(begin_window.descriptor());
  auto resize_peer = create_dx12_renderer(resize_window.descriptor());
  if (!first || !begin_peer || !resize_peer) return dx12_fail(2, "lazy cohort renderers unavailable");
  const auto first_before = (*first)->backend_info();
  const auto begin_before = (*begin_peer)->backend_info(), resize_before = (*resize_peer)->backend_info();
  if (!first_before.adapter_luid || first_before.adapter_luid != begin_before.adapter_luid ||
      first_before.adapter_luid != resize_before.adapter_luid)
    return dx12_fail(3, "lazy cohort adapters differ");
  if (!captured_clear(**first, {1, 0, 0, 1}, {255, 0, 0, 255}) ||
      !captured_clear(**begin_peer, {0, 1, 0, 1}, {0, 255, 0, 255}) ||
      !captured_clear(**resize_peer, {0, 0, 1, 1}, {0, 0, 255, 255}))
    return dx12_fail(4, "lazy cohort warmup failed");
  const auto support_before = (*resize_peer)->backdrop_blur_support();
  auto abandoned = (*resize_peer)->begin_frame();
  if (!abandoned) return dx12_fail(5, "lazy peer active frame unavailable");
  if (!dx12_remove_native_device(first_before)) return dx12_fail(6, "lazy cohort native removal failed");
  const auto retry = (*first)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(7, "lazy cohort initiator recovery failed");
  const auto first_after = (*first)->backend_info();
  if (first_after.device_generation != first_before.device_generation + 1 ||
      (*begin_peer)->backend_info().device_generation != begin_before.device_generation ||
      (*resize_peer)->backend_info().device_generation != resize_before.device_generation)
    return dx12_fail(8, "inactive peers rebuilt eagerly");
  const auto support_pending = (*resize_peer)->backdrop_blur_support();
  if (support_pending.supported != support_before.supported || support_pending.reason != support_before.reason)
    return dx12_fail(9, "native-only purge changed CPU capability metadata");
  // A peer may still record CPU commands after its native generation is purged.
  (*abandoned)->draw_rect({.rect = {{4, 4}, {16, 16}}, .color = {1, 0, 0, 1}});
  (*abandoned)->draw_backdrop_blur({.rect = {{4, 4}, {32, 32}}, .blur_radius = 4});
  abandoned->reset(); // Abandonment must release local ownership, not rebuild.
  auto first_active = (*first)->begin_frame();
  if (!first_active) return dx12_fail(10, "initiator not ready before lazy peer calls");
  const auto begin_retry = (*begin_peer)->begin_frame();
  if (begin_retry || begin_retry.error().code != ErrorCode::frame_retry_required ||
      (*begin_peer)->backend_info().device_generation != begin_before.device_generation + 1)
    return dx12_fail(11, "pending begin peer did not lazily rebuild once");
  if (!(*resize_peer)->resize({96, 48}, {2}) ||
      (*resize_peer)->backend_info().device_generation != resize_before.device_generation + 1)
    return dx12_fail(12, "pending resize peer did not complete lazy rebuild and resize");
  // Pending peers must not re-purge the initiator's already healthy new device.
  (*first_active)->clear({1, 0, 0, 1});
  if (!(*first_active)->request_pixel_capture() || !(*first_active)->present() ||
      !dx12_pixel((*first)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
      (*first)->backend_info().device_generation != first_after.device_generation)
    return dx12_fail(13, "lazy peer rebuild invalidated the initiator's new frame");
  if (!captured_clear(**begin_peer, {0, 1, 0, 1}, {0, 255, 0, 255}))
    return dx12_fail(14, "begin peer cannot render after lazy recovery");
  auto resized = (*resize_peer)->begin_frame();
  if (!resized) return dx12_fail(15, "resize peer not ready");
  (*resized)->clear({0, 0, 1, 1});
  (*resized)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 1}});
  if (!(*resized)->request_pixel_capture() || !(*resized)->present())
    return dx12_fail(16, "resize peer capture failed");
  const auto* pixels = (*resize_peer)->last_frame_pixels();
  if (!pixels || pixels->width != 96 || pixels->height != 48 ||
      !dx12_pixel(pixels, 20, 20, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 28, 12, {0, 0, 255, 255}))
    return dx12_fail(17, "lazy resize ignored target extent or DPI");
  return dx12_recovered_native_device_clean(first_after) ? 0 : dx12_fail(18, "lazy cohort debug errors");
}
