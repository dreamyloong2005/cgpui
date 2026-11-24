#include "../support/frame_wait_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_image_retention_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window, peer_window;
  auto renderer = create_dx12_renderer(window.descriptor());
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!deadline.done || !renderer || !peer) return dx12_fail(1, "retained recovery fixtures unavailable");
  const auto before = (*renderer)->backend_info(), peer_before = (*peer)->backend_info();
  if (!before.adapter_luid || before.adapter_luid != peer_before.adapter_luid)
    return dx12_fail(2, "retained recovery peers do not share the adapter");
  const auto red = retention_image(9840, {255, 0, 0, 255});
  const auto green = retention_image(9840, {0, 255, 0, 255});
  const auto asset = describe_image_asset(red);
  auto old_red = wait_for_test_frame(**renderer);
  if (!old_red) return dx12_fail(3, "old red image frame unavailable");
  (*old_red)->upload_image(red);
  if (!retention_present_image(**renderer, **old_red, asset, {{255, 0, 0, 255}, 4, 4}))
    return dx12_fail(4, "old red snapshot/pixels absent");
  auto old_green = wait_for_test_frame(**renderer);
  if (!old_green) return dx12_fail(5, "replacement green image frame unavailable");
  (*old_green)->upload_image(green);
  if (!retention_present_image(**renderer, **old_green, asset, {{0, 255, 0, 255}, 8, 4}))
    return dx12_fail(6, "replacement did not retain old finished-frame payload");
  auto peer_old = wait_for_test_frame(**peer);
  if (!peer_old) return dx12_fail(7, "peer image frame unavailable");
  (*peer_old)->upload_image(red);
  if (!retention_present_image(**peer, **peer_old, asset, {{255, 0, 0, 255}, 4, 4}))
    return dx12_fail(8, "peer image payload/pixels absent");
  const auto* borrowed = (*renderer)->last_frame_diagnostic_snapshot();
  const auto* peer_borrowed = (*peer)->last_frame_diagnostic_snapshot();
  const auto history = *borrowed, peer_history = *peer_borrowed;
  const auto unchanged = [&] {
    return (*renderer)->last_frame_diagnostic_snapshot() == borrowed && diagnostic_same(borrowed, history) &&
        (*peer)->last_frame_diagnostic_snapshot() == peer_borrowed && diagnostic_same(peer_borrowed, peer_history);
  };
  if (!dx12_remove_native_device(before)) return dx12_fail(9, "actual retention device removal failed");
  const auto retry = (*renderer)->begin_frame(); // Raw typed retry: not a successful sampled frame.
  if (retry || retry.error().code != ErrorCode::frame_retry_required || !unchanged() ||
      (*renderer)->backend_info().adapter_luid != before.adapter_luid ||
      (*renderer)->backend_info().device_generation != before.device_generation + 1 ||
      (*peer)->backend_info().device_generation != peer_before.device_generation || (*peer)->last_frame_pixels())
    return dx12_fail(10, "origin/peer retirement replaced diagnostic history or identity");
  const auto peer_retry = (*peer)->begin_frame();
  if (peer_retry || peer_retry.error().code != ErrorCode::frame_retry_required || !unchanged() ||
      (*peer)->backend_info().adapter_luid != peer_before.adapter_luid ||
      (*peer)->backend_info().device_generation != peer_before.device_generation + 1)
    return dx12_fail(11, "lazy peer rebuild changed historical CPU retention before successful Present");
  auto recovered = wait_for_test_frame(**renderer);
  if (!recovered || !retention_present_image(**renderer, **recovered, asset, {{0, 255, 0, 255}, 8, 4}))
    return dx12_fail(12, "native reupload lost retained old snapshots or counted a second CPU copy");
  auto warm = wait_for_test_frame(**renderer);
  if (!warm || !retention_present_image(**renderer, **warm, asset, {{0, 255, 0, 255}, 8, 0}))
    return dx12_fail(13, "warm recovered image repeated upload or changed live payload");
  if (!diagnostic_same((*peer)->last_frame_diagnostic_snapshot(), peer_history))
    return dx12_fail(14, "one renderer's new sample rewrote the peer's retained history");
  auto peer_frame = wait_for_test_frame(**peer);
  if (!peer_frame || !retention_present_image(**peer, **peer_frame, asset, {{255, 0, 0, 255}, 4, 4}))
    return dx12_fail(15, "peer CPU cache did not survive its lazy native reconstruction");
  old_red->reset();
  if (!retention_empty_frame(**renderer, 4))
    return dx12_fail(16, "releasing the old red finished frame did not reduce the next CPU census");
  old_green->reset(); recovered->reset(); warm->reset();
  auto invalidation = wait_for_test_frame(**renderer);
  if (!invalidation) return dx12_fail(17, "post-recovery invalidation frame unavailable");
  (*invalidation)->invalidate_image(asset.id);
  (*invalidation)->clear({0, 0, 1, 1});
  if (!(*invalidation)->request_pixel_capture() || !(*invalidation)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 32, 32, {0, 0, 255, 255}) ||
      !retention_sample(**renderer, 0, 0) || !retention_sample(**peer, 4) ||
      !dx12_recovered_native_device_clean((*renderer)->backend_info()))
    return dx12_fail(18, "final release/invalidation, per-renderer isolation or recovered debug state differs");
  return 0;
}
