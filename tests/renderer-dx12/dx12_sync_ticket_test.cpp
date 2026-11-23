#include "dx12_sync_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  int failures = 0;
  for (const bool fail_event : {true, false}) {
    Dx12SyncScript script;
    script.event_result = fail_event ? E_ACCESSDENIED : S_OK;
    auto state = dx12_sync_state(script);
    if (!state) return dx12_sync_fail(1, "native state setup failed");
    const auto first = dx12_wait_idle(*state);
    if (first || !state->submitted || script.signal_calls != 1) {
      ++failures;
      dx12_sync_fail(2, "event/wait failure must preserve pending work");
    }
    const auto submitted_ticket = script.last_signal;
    script.completion = submitted_ticket;
    const auto second = dx12_wait_idle(*state);
    if (!second || state->submitted || script.signal_calls != 1) {
      ++failures;
      dx12_sync_fail(3, fail_event
          ? "event failure discarded an already submitted completion ticket"
          : "wait failure discarded an already submitted completion ticket");
    }
    // No GPU work exists; prevent teardown policy from obscuring this verdict.
    state->submitted = false;
  }
  return failures ? 3 : 0;
}
