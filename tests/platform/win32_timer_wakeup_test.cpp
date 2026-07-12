#include "win32_timer_wakeup_internal.hpp"

int main() {
  MSG message{};
  (void)PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
  cgpui::Win32TimerWakeup timer;
  timer.set_thread_id(GetCurrentThreadId());
  timer.request(10);
  const DWORD ready = MsgWaitForMultipleObjects(
      0, nullptr, FALSE, 1'000, QS_POSTMESSAGE);
  if (ready != WAIT_OBJECT_0 ||
      !PeekMessageW(
          &message,
          nullptr,
          cgpui::cgpui_wakeup_message,
          cgpui::cgpui_wakeup_message,
          PM_REMOVE)) {
    return 1;
  }

  timer.request(100);
  timer.cancel();
  const DWORD cancelled = MsgWaitForMultipleObjects(
      0, nullptr, FALSE, 150, QS_POSTMESSAGE);
  if (cancelled == WAIT_OBJECT_0 &&
      PeekMessageW(
          &message,
          nullptr,
          cgpui::cgpui_wakeup_message,
          cgpui::cgpui_wakeup_message,
          PM_REMOVE)) {
    return 2;
  }
  return 0;
}
