#include "win32_timer_wakeup_internal.hpp"

#include <iostream>
#include <type_traits>

namespace {
PTP_TIMER WINAPI checked_native_timer(PTP_TIMER_CALLBACK callback, void* context,
                                     PTP_CALLBACK_ENVIRON environment) {
  if (!callback || !context || environment) { SetLastError(ERROR_INVALID_PARAMETER); return nullptr; }
  return CreateThreadpoolTimer(callback, context, environment);
}
bool take_wakeup() {
  MSG message{};
  return PeekMessageW(&message, nullptr, cgpui::cgpui_wakeup_message,
                       cgpui::cgpui_wakeup_message, PM_REMOVE) != FALSE;
}
}

int main() {
  static_assert(!std::is_copy_constructible_v<cgpui::Win32TimerWakeup>);
  static_assert(!std::is_move_constructible_v<cgpui::Win32TimerWakeup>);
  MSG message{};
  (void)PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
  {
    cgpui::Win32TimerWakeup timer(&checked_native_timer);
    if (!timer.initialization_result()) return 1;
    timer.request_now();
    timer.request_now(); // Unattached immediate notifications coalesce once.
    timer.set_thread_id(GetCurrentThreadId());
    if (!take_wakeup() || take_wakeup()) return 2;
    timer.request(10);
    if (MsgWaitForMultipleObjects(0, nullptr, FALSE, 1'000, QS_POSTMESSAGE) != WAIT_OBJECT_0 ||
        !take_wakeup()) return 3;
    timer.request(100);
    timer.cancel();
    (void)MsgWaitForMultipleObjects(0, nullptr, FALSE, 150, QS_POSTMESSAGE);
    if (take_wakeup()) return 4;
    timer.set_thread_id(0);
    timer.request_now();
    timer.cancel(); // Cancelling clears an unattached pending wake too.
    timer.set_thread_id(GetCurrentThreadId());
    if (take_wakeup()) return 5;
    timer.request(100); // Scope exit disarms, drains callbacks, then closes.
  }
  (void)MsgWaitForMultipleObjects(0, nullptr, FALSE, 150, QS_POSTMESSAGE);
  if (take_wakeup()) return 6;
  std::cout << "native delayed timer, cancellation and pending-destructor control passed\n";
  return 0;
}
