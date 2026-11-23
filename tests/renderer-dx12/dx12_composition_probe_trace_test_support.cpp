#include "dx12_composition_probe_test_support.hpp"
#include <cstdio>

namespace cgpui_test {
void composition_probe_note(const char* stage, const char* detail) {
  std::printf("COMPOSITION_PROBE t_ms=%llu stage=%s detail=%s\n",
      static_cast<unsigned long long>(GetTickCount64()), stage, detail);
  std::fflush(stdout); // Keep evidence if the bounded process watchdog terminates a native hang.
}
bool composition_probe_hresult(const char* stage, HRESULT result) {
  std::printf("COMPOSITION_PROBE t_ms=%llu stage=%s hr=0x%08lX\n",
      static_cast<unsigned long long>(GetTickCount64()), stage, static_cast<unsigned long>(result));
  std::fflush(stdout);
  return SUCCEEDED(result);
}
HRESULT Dx12CompositionProbeApi::set_maximum_latency(IDXGISwapChain2* chain, UINT latency) noexcept {
  ++sets;
  const auto result = native.set_maximum_latency(chain, latency);
  composition_probe_hresult("SetMaximumFrameLatency", result);
  return result;
}
HANDLE Dx12CompositionProbeApi::latency_object(IDXGISwapChain2* chain) noexcept {
  ++gets;
  observed_handle = native.latency_object(chain);
  acquired += observed_handle != nullptr;
  // NULL does not provide a specified GetLastError contract.
  std::printf("COMPOSITION_PROBE stage=GetFrameLatencyWaitableObject nonnull=%d\n",
      observed_handle != nullptr);
  std::fflush(stdout);
  return observed_handle;
}
cgpui::Dx12FramePacingWaitResult Dx12CompositionProbeApi::poll(HANDLE handle) noexcept {
  ++polls;
  const auto result = native.poll(handle); // Strict forwarding; never replace status/error.
  ready += result.status == WAIT_OBJECT_0;
  pending += result.status == WAIT_TIMEOUT;
  if (polls == 1 || result.status != WAIT_TIMEOUT)
    std::printf("COMPOSITION_PROBE t_ms=%llu stage=native_poll poll=%u status=%lu error=%lu\n",
        static_cast<unsigned long long>(GetTickCount64()), polls, result.status, result.error);
  std::fflush(stdout);
  return result;
}
bool Dx12CompositionProbeApi::close(HANDLE handle) noexcept {
  ++closes;
  const bool result = native.close(handle);
  close_ok = close_ok && result;
  std::printf("COMPOSITION_PROBE stage=CloseHandle count=%u ok=%d\n", closes, result);
  std::fflush(stdout);
  return result;
}
bool Dx12CompositionProbe::invariant(bool value, const char* detail) {
  if (!value) {
    outcome = CompositionProbeExit::invalid;
    composition_probe_note("invariant_failed", detail);
  }
  return value;
}
bool Dx12CompositionProbe::result(const char* stage, const cgpui::Result<void>& value) {
  composition_probe_note(stage, value ? "ok" : value.error().message.c_str());
  if (!value) outcome = CompositionProbeExit::unmeasured;
  return bool(value);
}
bool Dx12CompositionProbe::native(const char* stage, HRESULT value) {
  if (composition_probe_hresult(stage, value)) return true;
  outcome = CompositionProbeExit::unmeasured;
  return false;
}
} // namespace cgpui_test
