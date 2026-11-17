#pragma once

#include "win32_system_appearance_internal.hpp"
#include "win32_window_backdrop_internal.hpp"

namespace cgpui {

inline constexpr const wchar_t* win32_appearance_monitor_class =
    L"CGPUIAppearanceMonitor";

// Owns one hidden top-level window that receives the broadcast appearance
// messages (message-only windows do not get broadcasts). Each relevant
// message re-reads the snapshot, refreshes CGPUI title bars on this thread,
// and forwards a SystemAppearanceChange to the registered callback.
class Win32AppearanceMonitor {
 public:
  explicit Win32AppearanceMonitor(HINSTANCE instance);
  ~Win32AppearanceMonitor();
  Win32AppearanceMonitor(const Win32AppearanceMonitor&) = delete;
  Win32AppearanceMonitor& operator=(const Win32AppearanceMonitor&) = delete;

  bool ensure_started();
  void set_callback(SystemAppearanceCallback callback);
  void handle(SystemAppearanceChangeSource source);

 private:
  static LRESULT CALLBACK window_proc(
      HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  SystemAppearanceCallback callback_;
  SystemAppearance last_appearance_;
};

} // namespace cgpui
