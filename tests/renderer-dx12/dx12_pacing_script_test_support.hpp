#pragma once

#include "../../src/renderer/dx12/dx12_frame_pacing_internal.hpp"
#include <cstdint>
#include <vector>

namespace cgpui_test {
struct Dx12PacingScript final : cgpui::Dx12FramePacingApi {
  HRESULT configured = S_OK;
  HANDLE acquired = reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(0x1234));
  cgpui::Dx12FramePacingWaitResult waited{WAIT_TIMEOUT, ERROR_SUCCESS};
  unsigned sets = 0, gets = 0, polls = 0;
  UINT requested_latency = 0;
  std::vector<HANDLE> closed;

  HRESULT set_maximum_latency(IDXGISwapChain2*, UINT latency) noexcept override;
  HANDLE latency_object(IDXGISwapChain2*) noexcept override;
  cgpui::Dx12FramePacingWaitResult poll(HANDLE handle) noexcept override;
  bool close(HANDLE handle) noexcept override;
};
bool pacing_is(const cgpui::Result<cgpui::Dx12FrameAdmission>& result,
               cgpui::Dx12FrameAdmission expected);
int pacing_fail(int code, const char* message);
} // namespace cgpui_test
