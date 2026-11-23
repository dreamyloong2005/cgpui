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
  api.remove_on_poll = initial; // Real RemoveDevice, precisely after prepare and before poll returns.
  auto retry = renderer->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required || !api.removal_ok ||
      api.polls != 1 || api.forwarded.closes != 1 || api.forwarded.sets != 2 ||
      api.forwarded.gets != 2 || renderer->backend_info().device_generation != initial.device_generation + 1 ||
      renderer->backend_info().adapter_luid != initial.adapter_luid)
    return dx12_fail(3, "poll-boundary real device loss did not preserve typed same-adapter recovery");
  api.waited = {WAIT_TIMEOUT, ERROR_SUCCESS};
  auto pending = renderer->begin_frame();
  if (pending || pending.error().code != ErrorCode::frame_not_ready || api.polls != 2)
    return dx12_fail(4, "replacement chain inherited old grant or failed state");
  api.waited = {WAIT_OBJECT_0, ERROR_SUCCESS};
  auto frame = renderer->begin_frame();
  if (!frame || !pacing_control_capture(**frame, *renderer) ||
      !pacing_begin_debug_clean(renderer->backend_info()))
    return dx12_fail(5, "recovered real renderer failed pixels/debug");
  frame->reset();
  renderer.reset();
  if (api.forwarded.closes != 2 || !api.forwarded.close_ok) return dx12_fail(6, "recovery handle lifetime failed");
  std::cout << "PACING_CONTROL scripted-wait real-RemoveDevice recovery PASS\n";
  return 0;
}
