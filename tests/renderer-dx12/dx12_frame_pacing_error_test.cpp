#include "dx12_pacing_script_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12PacingScript script;
  Dx12FramePacing pacing(script);
  script.configured = E_ACCESSDENIED;
  const auto failed_set = pacing.configure(nullptr, true);
  if (failed_set || script.sets != 1 || script.gets || !script.closed.empty() || pacing.try_admit())
    return pacing_fail(1, "failed SetMaximumFrameLatency adopted or closed a handle");
  script.configured = S_OK;
  script.acquired = nullptr;
  SetLastError(ERROR_ACCESS_DENIED);
  const auto null_handle = pacing.configure(nullptr, true);
  if (null_handle || null_handle.error().code != ErrorCode::renderer_initialization_failed ||
      null_handle.error().message != "DX12 waitable swapchain returned a null frame-latency handle" ||
      script.gets != 1 || !script.closed.empty())
    return pacing_fail(2, "null acquisition read stale LastError or acquired ownership");
  script.acquired = reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(0x4567));
  if (!pacing.configure(nullptr, true)) return pacing_fail(3, "failed configuration prevented clean retry");
  script.waited = {WAIT_TIMEOUT, ERROR_ACCESS_DENIED};
  if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::pending))
    return pacing_fail(4, "timeout became a native/device failure");
  script.waited = {WAIT_FAILED, ERROR_ACCESS_DENIED};
  SetLastError(ERROR_FILE_NOT_FOUND);
  const auto denied = pacing.try_admit();
  if (denied || denied.error().message.find("0x80070005") == std::string::npos)
    return pacing_fail(5, "WAIT_FAILED lost the seam's immediately captured error");
  script.waited = {WAIT_FAILED, ERROR_SUCCESS};
  const auto missing_error = pacing.try_admit();
  if (missing_error || missing_error.error().message.find("0x8007001F") == std::string::npos)
    return pacing_fail(6, "WAIT_FAILED with zero error became success");
  for (const DWORD status : {DWORD(WAIT_ABANDONED), DWORD(WAIT_IO_COMPLETION), DWORD(37)}) {
    script.waited = {status, ERROR_SUCCESS};
    if (pacing.try_admit()) return pacing_fail(7, "unexpected non-alertable status invented readiness");
  }
  script.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
  if (!pacing_is(pacing.try_admit(), Dx12FrameAdmission::ready))
    return pacing_fail(8, "failed probes poisoned the owner instead of reporting their errors");
  pacing.reset();
  if (script.closed.size() != 1) return pacing_fail(9, "partial outer initialization cleanup lost ownership");
  Dx12NativeFramePacingApi native;
  if (native.set_maximum_latency(nullptr, 1) != E_POINTER || native.latency_object(nullptr))
    return pacing_fail(10, "native seam accepted a missing real chain");
  SetLastError(ERROR_ACCESS_DENIED);
  const auto actual_invalid = native.poll(nullptr);
  return actual_invalid.status == WAIT_FAILED && actual_invalid.error == ERROR_INVALID_HANDLE
      ? 0 : pacing_fail(11, "real zero-time wait did not capture its immediate invalid-handle error");
}
