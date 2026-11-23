#include "dx12_sync_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct SpuriousWake final : Dx12SyncScript {
  unsigned wake_calls = 0;
  unsigned post_wake_reads = 0;
  bool incomplete_after_wake = false;
  bool completion_observed = false;
  SpuriousWake() {
    event_result = S_OK;
    wait_result = {WAIT_OBJECT_0, ERROR_SUCCESS};
  }
  UINT64 completed(ID3D12Fence&) noexcept override {
    if (!wake_calls) return 0;
    // The first post-wake query remains below the ticket, independently of
    // how many completion queries happened before the event was awaited.
    if (++post_wake_reads == 1) {
      incomplete_after_wake = true;
      return 0;
    }
    completion_observed = true;
    return last_signal;
  }
  Dx12SynchronizationWaitResult wait(HANDLE) noexcept override {
    ++wake_calls;
    return {WAIT_OBJECT_0, ERROR_SUCCESS};
  }
};
}

int main() {
  SpuriousWake script;
  auto state = dx12_sync_state(script);
  if (!state) return dx12_sync_fail(1, "native state setup failed");
  const auto result = dx12_wait_idle(*state);
  const bool safe = script.completion_observed
      ? (result && !state->submitted)
      : (!result && state->submitted);
  const bool one_ticket = script.signal_calls == 1;
  state->submitted = false; // This test has never submitted native GPU work.
  if (!script.wake_calls || !script.incomplete_after_wake) {
    return dx12_sync_fail(4, "test did not exercise an incomplete post-wake ticket");
  }
  if (!safe) return dx12_sync_fail(2, "event wake reported idle before its fence completed");
  if (!one_ticket) return dx12_sync_fail(3, "event wake replaced the pending ticket");
  return 0;
}
