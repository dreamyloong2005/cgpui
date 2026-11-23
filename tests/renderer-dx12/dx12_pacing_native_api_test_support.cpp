#include "dx12_pacing_native_test_support.hpp"

namespace cgpui_test {
HRESULT Dx12PacingForwardApi::set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept {
  ++sets;
  return native.set_maximum_latency(chain, latency);
}
HANDLE Dx12PacingForwardApi::latency_object(IDXGISwapChain2* chain) noexcept {
  ++gets;
  observed_handle = native.latency_object(chain);
  return observed_handle;
}
cgpui::Dx12FramePacingWaitResult Dx12PacingForwardApi::poll(HANDLE handle) noexcept {
  ++polls;
  const auto result = native.poll(handle); // Never substitute native status/error.
  ready += result.status == WAIT_OBJECT_0;
  pending += result.status == WAIT_TIMEOUT;
  return result;
}
bool Dx12PacingForwardApi::close(HANDLE handle) noexcept {
  ++closes;
  const bool result = native.close(handle);
  close_ok = close_ok && result;
  return result;
}
} // namespace cgpui_test
