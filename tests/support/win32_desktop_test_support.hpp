#pragma once

namespace cgpui_test {

enum class DesktopTestAccess { none, exclusive };
inline constexpr unsigned long desktop_test_queue_timeout_ms = 300000;

// Cooperative, session-local ownership of visible test windows and real input.
// Acquire before starting a test watchdog. Destroy on the acquiring thread,
// after every visible HWND; never keep a lease while awaiting a leased child.
class Win32DesktopTestLease {
 public:
  explicit Win32DesktopTestLease(DesktopTestAccess access = DesktopTestAccess::exclusive);
  ~Win32DesktopTestLease();
  Win32DesktopTestLease(const Win32DesktopTestLease&) = delete;
  Win32DesktopTestLease& operator=(const Win32DesktopTestLease&) = delete;
  [[nodiscard]] bool ready() const noexcept;

 private:
  void* mutex_ = nullptr;
  bool acquired_ = false;
  bool required_ = false;
};

} // namespace cgpui_test
