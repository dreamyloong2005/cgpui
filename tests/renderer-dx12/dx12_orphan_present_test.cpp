#include "dx12_pacing_begin_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "dx12_diagnostics_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
int run_case(bool finished_first) {
  Dx12Window window, peer_window; // Native windows outlive every renderer/frame.
  Dx12PacingForwardApi api, peer_api;
  if (!window.hwnd || !peer_window.hwnd) return dx12_fail(1, "orphan HWND setup failed");
  auto created = dx12_create_renderer(window.descriptor(), api);
  auto peer_created = dx12_create_renderer(peer_window.descriptor(), peer_api);
  if (!created || !peer_created) return dx12_fail(2, "actual orphan/peer renderer creation failed");
  auto renderer = std::move(*created), peer = std::move(*peer_created);
  const auto info = renderer->backend_info(), peer_info = peer->backend_info();
  const char* selected = std::getenv("CGPUI_DX12_ADAPTER");
  const bool warp = selected && std::string_view(selected) == "warp";
  if (info.backend != RendererBackend::dx12 || info.selection != RendererBackendSelection::dx12 ||
      !info.adapter_luid || (warp && !info.software_adapter) ||
      info.adapter_luid != peer_info.adapter_luid || !info.device_generation ||
      !info.fallback_reason.empty() || api.sets != 1 || api.gets != 1 || !api.observed_handle || api.closes)
    return dx12_fail(3, "orphan fixture lacked explicit real DX12/native pacing identity");
  auto finished = pacing_begin_bounded(*renderer, api);
  if (!finished) return dx12_fail(4, "orphan warmup frame unavailable");
  (*finished)->clear({1, 0, 0, 1});
  if (!(*finished)->request_pixel_capture() || !(*finished)->present() ||
      !dx12_pixel(renderer->last_frame_pixels(), 32, 32, {255, 0, 0, 255}))
    return dx12_fail(5, "actual warmup Present/readback failed before orphan oracle");
  {
    auto seed = pacing_begin_bounded(*peer, peer_api);
    if (!seed) return dx12_fail(6, "peer warmup frame unavailable");
    (*seed)->clear({0, 0, 1, 1});
    if (!(*seed)->request_pixel_capture() || !(*seed)->present() ||
        !dx12_pixel(peer->last_frame_pixels(), 32, 32, {0, 0, 255, 255}))
      return dx12_fail(7, "peer warmup pixels failed");
  }
  const auto* peer_history = peer->last_frame_diagnostic_snapshot();
  if (!peer_history) return dx12_fail(8, "peer warmup lacked diagnostics");
  const auto saved_history = *peer_history;
  auto active_peer = pacing_begin_bounded(*peer, peer_api);
  auto orphan = pacing_begin_bounded(*renderer, api);
  if (!active_peer || !orphan || !(*orphan)->request_pixel_capture())
    return dx12_fail(9, "real admitted orphan/peer frame unavailable");
  const auto polls = api.polls, peer_polls = peer_api.polls;
  renderer.reset();
  if (api.closes || !IsWindow(window.hwnd)) return dx12_fail(10, "facade destruction retired native state");
  const auto requested_after_owner = (*orphan)->request_pixel_capture();
  (*orphan)->clear({0, 1, 0, 1});
  (*orphan)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 1}});
  (*orphan)->draw_text(diagnostic_text("orphan", 14, {4, 20}));
  const auto presented = (*orphan)->present(); // Raw semantic oracle; never retry Present.
  std::cout << "ORPHAN_PRESENT finished_first=" << finished_first << " success=" << bool(presented)
            << " adapter=" << info.adapter_name << " generation=" << info.device_generation << std::endl;
  if (!presented) return dx12_fail(40, "FAIL_ORPHAN_PRESENT: acquired frame must finish after facade destruction");
  if (!requested_after_owner) return dx12_fail(19, "active orphan capture request must remain valid");
  if ((*orphan)->present() || (*orphan)->request_pixel_capture() || (*finished)->present())
    return dx12_fail(11, "finished orphan regained Present/capture permission");
  if (api.sets != 1 || api.gets != 1 || api.polls != polls || api.closes ||
      peer_api.polls != peer_polls || peer_api.closes ||
      peer->backend_info().device_generation != peer_info.device_generation ||
      peer->last_frame_diagnostic_snapshot() != peer_history || !diagnostic_same(peer_history, saved_history) ||
      !dx12_pixel(peer->last_frame_pixels(), 32, 32, {0, 0, 255, 255}) ||
      !pacing_begin_debug_clean(info))
    return dx12_fail(12, "orphan Present changed epoch, pacing or independent peer history");
  if (finished_first) finished->reset(); else orphan->reset();
  if (api.closes) return dx12_fail(13, "first frame destruction closed the surviving frame's native state");
  orphan->reset(); finished->reset();
  if (api.closes != 1 || !api.close_ok || api.sets != 1 || api.gets != 1 || api.polls != polls ||
      peer_api.closes || peer_api.polls != peer_polls)
    return dx12_fail(14, "last orphan release did not close exactly its own latency handle once");
  const auto overlap = peer->begin_frame();
  if (overlap || overlap.error().code != ErrorCode::invalid_argument)
    return dx12_fail(15, "orphan cleanup unlocked the unrelated active peer frame");
  (*active_peer)->clear({0, 1, 0, 1});
  if (!(*active_peer)->request_pixel_capture() || !(*active_peer)->present() ||
      !dx12_pixel(peer->last_frame_pixels(), 32, 32, {0, 255, 0, 255}) ||
      !pacing_begin_debug_clean(peer_info)) return dx12_fail(16, "live peer failed after orphan cleanup");
  active_peer->reset(); peer.reset();
  return peer_api.closes == 1 && peer_api.close_ok ? 0 : dx12_fail(17, "peer native epoch leaked or double closed");
}
}
int main() {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done || !pacing_begin_debug_enable()) return dx12_fail(18, "orphan debug/watchdog unavailable");
  int first_failure = 0;
  for (const bool finished_first : {false, true}) {
    const int result = run_case(finished_first);
    if (result && !first_failure) first_failure = result;
  }
  return first_failure;
}
