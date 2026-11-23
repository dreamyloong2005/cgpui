#include "dx12_pacing_control_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include <utility>

namespace cgpui_test {
HRESULT Dx12PacingControlApi::set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept {
  return forwarded.set_maximum_latency(chain, latency);
}
HANDLE Dx12PacingControlApi::latency_object(IDXGISwapChain2* chain) noexcept {
  return forwarded.latency_object(chain);
}
cgpui::Dx12FramePacingWaitResult Dx12PacingControlApi::poll(HANDLE handle) noexcept {
  ++polls;
  if (handle != forwarded.observed_handle) return {WAIT_FAILED, ERROR_INVALID_HANDLE};
  if (auto info = std::exchange(remove_on_poll, std::nullopt))
    removal_ok = dx12_remove_native_device(*info);
  return waited;
}
bool Dx12PacingControlApi::close(HANDLE handle) noexcept { return forwarded.close(handle); }
bool pacing_control_capture(cgpui::RenderFrame& frame, cgpui::Renderer& renderer) {
  frame.clear({0, 0, 1, 1});
  frame.draw_rect({.rect = {{8, 8}, {24, 24}}, .color = {1, 0, 0, 1}});
  return frame.request_pixel_capture() && frame.present() &&
      dx12_pixel(renderer.last_frame_pixels(), 16, 16, {255, 0, 0, 255}) &&
      dx12_pixel(renderer.last_frame_pixels(), 48, 48, {0, 0, 255, 255});
}
} // namespace cgpui_test
