#pragma once

#include "dx12_native_internal.hpp"

namespace cgpui {
struct Dx12FramePacingWaitResult {
  DWORD status = WAIT_FAILED;
  DWORD error = ERROR_SUCCESS;
};

// Private native seam; a borrowed scripted/forwarding API must outlive its owner.
// The chain pointer is borrowed only for configuration, never retained here.
class Dx12FramePacingApi {
 public:
  virtual ~Dx12FramePacingApi() = default;
  virtual HRESULT set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept = 0;
  virtual HANDLE latency_object(IDXGISwapChain2* chain) noexcept = 0;
  virtual Dx12FramePacingWaitResult poll(HANDLE handle) noexcept = 0;
  virtual bool close(HANDLE handle) noexcept = 0;
};

class Dx12NativeFramePacingApi final : public Dx12FramePacingApi {
 public:
  HRESULT set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept override;
  HANDLE latency_object(IDXGISwapChain2* chain) noexcept override;
  Dx12FramePacingWaitResult poll(HANDLE handle) noexcept override;
  bool close(HANDLE handle) noexcept override;
};

Dx12FramePacingApi& dx12_native_frame_pacing_api() noexcept;
} // namespace cgpui
