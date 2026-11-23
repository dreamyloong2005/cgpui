#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "dx12_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_pacing_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_internal.hpp"

namespace cgpui_test {
struct Dx12PacingForwardApi final : cgpui::Dx12FramePacingApi {
  cgpui::Dx12NativeFramePacingApi native;
  unsigned sets = 0, gets = 0, polls = 0, closes = 0;
  unsigned ready = 0, pending = 0;
  bool close_ok = true;
  HANDLE observed_handle = nullptr; // Non-owning; never waited/closed separately.
  HRESULT set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept override;
  HANDLE latency_object(IDXGISwapChain2* chain) noexcept override;
  cgpui::Dx12FramePacingWaitResult poll(HANDLE handle) noexcept override;
  bool close(HANDLE handle) noexcept override;
};
struct Dx12PacingNativeFixture {
  Dx12Window window;
  Dx12PacingForwardApi api;
  std::shared_ptr<cgpui::Dx12State> state;
  cgpui::Dx12FramePacing pacing;
  Dx12PacingNativeFixture();
  ~Dx12PacingNativeFixture();
  bool initialize();
  bool create_chain();
  bool await_admission();
  std::unique_ptr<cgpui::Dx12Frame> begin_admitted();
  bool present_pixels(cgpui::Dx12Frame& frame, bool green);
  bool debug_clean() const;
};
} // namespace cgpui_test
