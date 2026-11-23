#include "dx12_recovery_test_support.hpp"
#include "dx12_diagnostics_support.hpp"

#include <limits>

using namespace cgpui;
using namespace cgpui_test;

namespace {
bool paint(Renderer& renderer, unsigned rectangles) {
  auto frame = renderer.begin_frame();
  if (!frame) return false;
  (*frame)->clear({0, 0, 1, 1});
  for (unsigned index = 0; index != rectangles; ++index)
    (*frame)->draw_rect({.rect = {{4.0F + float(index) * 16, 4}, {12, 12}}, .color = {1, 0, 0, 1}});
  return (*frame)->request_pixel_capture() && (*frame)->present() &&
      dx12_pixel(renderer.last_frame_pixels(), 60, 40, {0, 0, 255, 255}) &&
      (!rectangles || dx12_pixel(renderer.last_frame_pixels(), 8, 8, {255, 0, 0, 255})) &&
      diagnostic_work(renderer.last_frame_diagnostic_snapshot(), rectangles, rectangles, rectangles);
}
}

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window, peer_window;
  auto renderer = create_dx12_renderer(window.descriptor());
  auto peer = create_dx12_renderer(peer_window.descriptor());
  if (!deadline.done || !renderer || !peer) return dx12_fail(1, "diagnostic lifetime fixtures unavailable");
  const auto origin_info = (*renderer)->backend_info(), peer_info = (*peer)->backend_info();
  if (!origin_info.adapter_luid || origin_info.adapter_luid != peer_info.adapter_luid)
    return dx12_fail(2, "diagnostic recovery peers do not share the native adapter");
  {
    auto abandoned = (*renderer)->begin_frame();
    if (!abandoned) return dx12_fail(3, "initial diagnostic frame unavailable");
    (*abandoned)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 1}});
  }
  if ((*renderer)->last_frame_diagnostic_snapshot() || !paint(**renderer, 1) || !paint(**peer, 2))
    return dx12_fail(4, "abandonment published diagnostics or successful warmup did not");
  const auto* borrowed = (*renderer)->last_frame_diagnostic_snapshot();
  const auto* peer_borrowed = (*peer)->last_frame_diagnostic_snapshot();
  const auto saved = *borrowed, peer_saved = *peer_borrowed;
  const auto retained = [&] {
    return (*renderer)->last_frame_diagnostic_snapshot() == borrowed && diagnostic_same(borrowed, saved) &&
        (*peer)->last_frame_diagnostic_snapshot() == peer_borrowed && diagnostic_same(peer_borrowed, peer_saved);
  };
  {
    auto abandoned = (*renderer)->begin_frame();
    if (!abandoned) return dx12_fail(5, "post-success diagnostic frame unavailable");
    (*abandoned)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {0, 1, 0, 1}});
    if ((*renderer)->begin_frame() || !retained()) return dx12_fail(6, "active/failed begin changed last successful diagnostics");
  }
  auto invalid = (*renderer)->begin_frame();
  if (!invalid) return dx12_fail(7, "invalid diagnostic frame unavailable");
  (*invalid)->draw_rect({.rect = {{4, 4}, {8, 8}}, .color = {1, 0, 0, 1}});
  (*invalid)->clear({std::numeric_limits<float>::quiet_NaN(), 0, 0, 1});
  const auto rejected = (*invalid)->present();
  if (rejected || rejected.error().code != ErrorCode::invalid_argument || (*invalid)->present() || !retained())
    return dx12_fail(8, "failed/repeated present overwrote last successful diagnostics");
  if (!(*renderer)->resize({96, 64}, {1}) || !retained() ||
      (*renderer)->resize({0, 64}, {1}) || !retained() || !(*renderer)->resize({64, 64}, {1}) || !retained())
    return dx12_fail(9, "resize or invalid resize changed retained diagnostic CPU data");
  if (!dx12_remove_native_device(origin_info)) return dx12_fail(10, "diagnostic native device removal unverified");
  const auto retry = (*renderer)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required || !retained() ||
      (*renderer)->backend_info().device_generation != origin_info.device_generation + 1 ||
      (*peer)->backend_info().device_generation != peer_info.device_generation || (*peer)->last_frame_pixels())
    return dx12_fail(11, "native recovery or pending-peer retirement invalidated a borrowed snapshot");
  const auto peer_retry = (*peer)->begin_frame();
  if (peer_retry || peer_retry.error().code != ErrorCode::frame_retry_required || !retained() ||
      (*peer)->backend_info().device_generation != peer_info.device_generation + 1)
    return dx12_fail(12, "lazy peer rebuild replaced last successful diagnostics before presentation");
  if (!paint(**renderer, 0) || !diagnostic_same((*peer)->last_frame_diagnostic_snapshot(), peer_saved) ||
      (*renderer)->last_frame_diagnostic_snapshot()->work.submitted_work.command_count != 0)
    return dx12_fail(13, "successful next frame did not publish only its own new diagnostic snapshot");
  if (!paint(**peer, 1) || (*peer)->last_frame_diagnostic_snapshot()->work.submitted_work.command_count != 1)
    return dx12_fail(14, "recovered peer did not publish its next successful frame");
  return 0;
}
