#include "dx12_frame_pacing_native_internal.hpp"

namespace cgpui {
HRESULT Dx12NativeFramePacingApi::set_maximum_latency(
    IDXGISwapChain2* chain, UINT latency) noexcept {
  return chain ? chain->SetMaximumFrameLatency(latency) : E_POINTER;
}
HANDLE Dx12NativeFramePacingApi::latency_object(IDXGISwapChain2* chain) noexcept {
  return chain ? chain->GetFrameLatencyWaitableObject() : nullptr;
}
Dx12FramePacingWaitResult Dx12NativeFramePacingApi::poll(HANDLE handle) noexcept {
  const DWORD status = WaitForSingleObjectEx(handle, 0, FALSE);
  const DWORD error = status == WAIT_FAILED ? GetLastError() : ERROR_SUCCESS;
  return {.status = status, .error = error};
}
bool Dx12NativeFramePacingApi::close(HANDLE handle) noexcept {
  return CloseHandle(handle) != FALSE;
}
Dx12FramePacingApi& dx12_native_frame_pacing_api() noexcept {
  static Dx12NativeFramePacingApi api;
  return api;
}
} // namespace cgpui
