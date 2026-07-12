#pragma once

#include "win32_internal.hpp"

#include <atomic>
#include <cstdint>

namespace cgpui {

class Win32TimerWakeup {
 public:
  Win32TimerWakeup();
  ~Win32TimerWakeup();

  Win32TimerWakeup(const Win32TimerWakeup&) = delete;
  Win32TimerWakeup& operator=(const Win32TimerWakeup&) = delete;

  void set_thread_id(DWORD thread_id);
  void request_now();
  void request(std::uint64_t delay_ms);
  void cancel();

 private:
  static void CALLBACK timer_callback(
      PTP_CALLBACK_INSTANCE,
      void* context,
      PTP_TIMER);
  void post_wakeup();

  PTP_TIMER timer_ = nullptr;
  std::atomic<DWORD> thread_id_ = 0;
  std::atomic_bool pending_ = false;
};

} // namespace cgpui
