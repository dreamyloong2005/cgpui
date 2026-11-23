#include "dx12_pacing_control_test_support.hpp"
#include "dx12_recovery_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  Dx12PacingControlApi api;
  if (!deadline.done || !window.hwnd || !pacing_begin_debug_enable()) return dx12_fail(1, "setup failed");
  auto created = dx12_create_renderer(window.descriptor(), api);
  if (!created) return dx12_fail(2, created.error().message.c_str());
  auto renderer = std::move(*created);
  const auto initial = renderer->backend_info();
  api.waited = {WAIT_FAILED, ERROR_INVALID_HANDLE};
  const auto expected = dx12_error(HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE), "frame admission wait");
  auto failed = renderer->begin_frame();
  if (failed || failed.error().code != expected.code || failed.error().message != expected.message ||
      api.polls != 1 || renderer->backend_info().device_generation != initial.device_generation)
    return dx12_fail(3, "live poll failure lost actual error or fabricated recovery");
  auto terminal = renderer->begin_frame();
  if (terminal || terminal.error().code == ErrorCode::frame_not_ready || api.polls != 1)
    return dx12_fail(4, "failed live renderer continued admission");
  renderer.reset();
  if (api.forwarded.closes != 1 || !api.forwarded.close_ok) return dx12_fail(5, "failed owner leaked handle");
  std::cout << "PACING_CONTROL scripted-invalid-handle native-renderer terminal PASS\n";
  return 0;
}
