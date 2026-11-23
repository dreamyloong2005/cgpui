#include "dx12_sync_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;

int main() {
  int failures = 0;
  for (const DWORD captured_error : {ERROR_SUCCESS, ERROR_ACCESS_DENIED}) {
    Dx12SyncScript script;
    script.event_result = S_OK;
    script.wait_result = {WAIT_FAILED, captured_error};
    auto state = dx12_sync_state(script);
    if (!state) return dx12_sync_fail(1, "native state setup failed");
    SetLastError(ERROR_FILE_NOT_FOUND); // Must not replace the captured wait error.
    const auto result = dx12_wait_idle(*state);
    const auto* expected = captured_error == ERROR_SUCCESS ? "8007001F" : "80070005";
    if (result || result.error().message.find(expected) == std::string::npos || !state->submitted) {
      ++failures;
      dx12_sync_fail(2, "wait failure needs a non-success HRESULT using its captured error");
    }
    state->submitted = false; // No native GPU submission occurred.
  }
  return failures ? 2 : 0;
}
