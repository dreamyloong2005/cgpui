#include "win32_timer_test_support.hpp"

#include <iostream>

int main() {
  MSG message{};
  (void)PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE); // Create this real thread queue.
  for (const bool attach_first : {false, true}) {
    cgpui::Win32TimerWakeup timer(&cgpui_test::reject_timer<ERROR_NOT_ENOUGH_MEMORY>);
    if (timer.initialization_result()) return 1;
    if (attach_first) timer.set_thread_id(GetCurrentThreadId());
    timer.request(60'000); // Allocation failure is never a successful immediate delayed wake.
    if (!attach_first) timer.set_thread_id(GetCurrentThreadId());
    if (PeekMessageW(&message, nullptr, cgpui::cgpui_wakeup_message,
                     cgpui::cgpui_wakeup_message, PM_REMOVE)) {
      std::cerr << "failed future timer posted an immediate wake; attached=" << attach_first << '\n';
      return 2;
    }
    timer.cancel();
    timer.set_thread_id(0);
  }
  return 0;
}
