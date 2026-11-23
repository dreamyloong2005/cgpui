#pragma once

#include "dx12_pacing_begin_test_support.hpp"
#include "../../src/renderer/dx12/dx12_renderer_creation_internal.hpp"
#include <optional>

namespace cgpui_test {
// Only the wait result is scripted. Configuration, handle ownership, renderer,
// swapchain, draws and captures remain native. This is NOT native readiness proof.
struct Dx12PacingControlApi final : cgpui::Dx12FramePacingApi {
  Dx12PacingForwardApi forwarded;
  cgpui::Dx12FramePacingWaitResult waited{WAIT_TIMEOUT, ERROR_SUCCESS};
  unsigned polls = 0;
  std::optional<cgpui::RendererBackendInfo> remove_on_poll;
  bool removal_ok = false;
  HRESULT set_maximum_latency(IDXGISwapChain2*, UINT) noexcept override;
  HANDLE latency_object(IDXGISwapChain2*) noexcept override;
  cgpui::Dx12FramePacingWaitResult poll(HANDLE) noexcept override;
  bool close(HANDLE) noexcept override;
};
bool pacing_control_capture(cgpui::RenderFrame&, cgpui::Renderer&);
} // namespace cgpui_test
