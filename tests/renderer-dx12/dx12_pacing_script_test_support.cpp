#include "dx12_pacing_script_test_support.hpp"
#include <iostream>

namespace cgpui_test {
HRESULT Dx12PacingScript::set_maximum_latency(IDXGISwapChain2*, UINT latency) noexcept {
  ++sets;
  requested_latency = latency;
  return configured;
}
HANDLE Dx12PacingScript::latency_object(IDXGISwapChain2*) noexcept {
  ++gets;
  return acquired;
}
cgpui::Dx12FramePacingWaitResult Dx12PacingScript::poll(HANDLE handle) noexcept {
  ++polls;
  return handle == acquired ? waited
      : cgpui::Dx12FramePacingWaitResult{WAIT_FAILED, ERROR_INVALID_HANDLE};
}
bool Dx12PacingScript::close(HANDLE handle) noexcept {
  closed.push_back(handle);
  return true;
}
bool pacing_is(const cgpui::Result<cgpui::Dx12FrameAdmission>& result,
               cgpui::Dx12FrameAdmission expected) {
  return result && *result == expected;
}
int pacing_fail(int code, const char* message) {
  std::cerr << "DX12 pacing core: " << message << '\n';
  return code;
}
} // namespace cgpui_test
