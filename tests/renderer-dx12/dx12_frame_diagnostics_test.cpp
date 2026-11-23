#include "../support/frame_wait_test_support.hpp"
#include "dx12_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12Window window;
  if (!window.hwnd) return dx12_fail(1, "diagnostics native window unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  if ((*renderer)->backend_info().backend != RendererBackend::dx12 ||
      (*renderer)->last_frame_diagnostic_snapshot() != nullptr)
    return dx12_fail(3, "new DX12 renderer must not expose a completed-frame diagnostic snapshot");
  auto frame = cgpui_test::wait_for_test_frame(**renderer);
  if (!frame) return dx12_fail(4, frame.error().message.c_str());
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_rect({.rect = {{8, 8}, {16, 16}}, .color = {1, 0, 0, 1}});
  (*frame)->draw_rect({.rect = {{32, 8}, {16, 16}}, .color = {0, 1, 0, 1}});
  if (!(*frame)->request_pixel_capture()) return dx12_fail(5, "diagnostics capture request failed");
  const auto presented = (*frame)->present();
  if (!presented) return dx12_fail(6, presented.error().message.c_str());
  const auto* pixels = (*renderer)->last_frame_pixels();
  if (!pixels || pixels->width != 64 || pixels->height != 64 ||
      !dx12_pixel(pixels, 12, 12, {255, 0, 0, 255}) ||
      !dx12_pixel(pixels, 36, 12, {0, 255, 0, 255}) ||
      !dx12_pixel(pixels, 56, 40, {0, 0, 255, 255}))
    return dx12_fail(7, "diagnostic frame did not actually draw both rectangles");
  const auto* snapshot = (*renderer)->last_frame_diagnostic_snapshot();
  if (!snapshot) return dx12_fail(8, "successful DX12 frame has no public diagnostic snapshot");
  // Commands/draws describe the two submitted primitives, not the clear,
  // capture copy, present operation, vertices, or implementation draw calls.
  if (snapshot->work.planned_work.command_count != 2 || snapshot->work.submitted_work.command_count != 2 ||
      snapshot->work.planned_work.draw_count != 2 || snapshot->work.submitted_work.draw_count != 2 ||
      snapshot->planned_draws.solid_rect_count != 2 || snapshot->submitted_draws.solid_rect_count != 2 ||
      snapshot->planned_draws.total_count != 2 || snapshot->submitted_draws.total_count != 2 ||
      snapshot->planned_draws.saturated || snapshot->submitted_draws.saturated ||
      !snapshot->dropped_resources.resources.empty())
    return dx12_fail(9, "public frame diagnostic primitive/command submission counts differ");
  std::cout << "DX12_FRAME_DIAGNOSTICS planned=2 submitted=2 solid-rects=2 pixels=64x64\n";
  return 0;
}
