#include "../support/frame_wait_test_support.hpp"
#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_dred_native_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  dred_capture_requests();
  Dx12Window window, peer_window;
  Dx12PacingForwardApi api;
  DredNativeReadout scalar;
  if (!deadline.done || !window.hwnd || !peer_window.hwnd || !pacing_begin_debug_enable())
    return dx12_fail(1, "removed orphan fixture unavailable");
  auto created = dx12_create_renderer(window.descriptor(), api);
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!created || !peer) return dx12_fail(2, "removed orphan real renderer creation failed");
  auto renderer = std::move(*created);
  const auto before = renderer->backend_info(), peer_before = (*peer)->backend_info();
  if (!dred_record_identity(before, "orphan-initial") ||
      !dred_record_identity(peer_before, "orphan-peer-initial")) return dx12_fail(13, "orphan native identity/output failed");
  if (!before.adapter_luid || before.adapter_luid != peer_before.adapter_luid)
    return dx12_fail(3, "removed orphan and peer must share an adapter");
  const auto weak = dred_attach_public_readout(before, window.hwnd, scalar);
  if (weak.expired()) return dx12_fail(11, "orphan scalar readout attachment" );
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!orphan || !(*orphan)->request_pixel_capture()) return dx12_fail(4, "removed orphan frame unavailable");
  renderer.reset();
  const auto loss = dred_remove_actual(before, dx12_recovery_debug_requested());
  if (!loss.valid) return dx12_fail(5, "actual orphan native removal failed" );
  const auto polls = api.polls;
  const auto rejected = (*orphan)->present(); // Orphan is first observer, before any live-peer recovery.
  if (rejected || rejected.error().code == ErrorCode::frame_retry_required ||
      rejected.error().code == ErrorCode::frame_not_ready ||
      api.sets != 1 || api.gets != 1 || api.polls != polls || api.closes != 1 || !api.close_ok)
    return dx12_fail(6, "removed orphan rebuilt, advertised retry or failed to retire its native epoch");
  if (scalar.calls != 1 || !dred_current_readout(scalar, loss, before.device_generation))
    return dx12_fail(12, "orphan-first common scalar publication absent" );
  orphan->reset();
  if (api.closes != 1) return dx12_fail(7, "removed orphan final destruction closed its epoch twice");
  const auto retry = (*peer)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required ||
      (*peer)->backend_info().device_generation != peer_before.device_generation + 1 ||
      (*peer)->backend_info().adapter_luid != peer_before.adapter_luid)
    return dx12_fail(8, "orphan retirement blocked the live peer's own recovery");
  if (!dred_record_identity((*peer)->backend_info(), "orphan-peer-recovered"))
    return dx12_fail(13, "orphan peer identity/output failed");
  auto frame = wait_for_test_frame(**peer);
  if (!frame) return dx12_fail(9, "live peer post-removal frame unavailable");
  (*frame)->clear({0, 1, 0, 1});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*peer)->last_frame_pixels(), 32, 32, {0, 255, 0, 255}) ||
      !pacing_begin_debug_clean((*peer)->backend_info()))
    return dx12_fail(10, "live peer pixels/debug failed after orphan-first removal retirement");
  frame->reset(); peer->reset();
  return dred_flush_output() ? 0 : dx12_fail(14, "orphan evidence output flush failed");
}
