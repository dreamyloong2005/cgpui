#pragma once

#include "win32_timer_wakeup_internal.hpp"

namespace cgpui_test {

// Per-call stateless failure only. Never returns a fabricated successful timer.
template <DWORD Code>
PTP_TIMER WINAPI reject_timer(PTP_TIMER_CALLBACK callback, void* context,
                             PTP_CALLBACK_ENVIRON environment) {
  SetLastError(callback && context && !environment ? Code : ERROR_INVALID_PARAMETER);
  return nullptr;
}

} // namespace cgpui_test
